// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingDSOnlineSession.h"

#include "MatchmakingDSLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineUserInterface.h"

void UMatchmakingDSOnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// Game Session delegates
	GetABSessionInt()->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	GetABSessionInt()->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Matchmaking delegates
	GetSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);
}

void UMatchmakingDSOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	GetSessionInt()->OnMatchmakingCompleteDelegates.RemoveAll(this);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.RemoveAll(this);
}

#pragma region "Game Session Essentials"
void UMatchmakingDSOnlineSession::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("User interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	// Some data does not exist in cache, query everything
	else
	{
		// Bind delegate
		if (OnQueryUserInfoCompleteDelegateHandle.IsValid())
		{
			GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
			OnQueryUserInfoCompleteDelegateHandle.Reset();
		}
		OnQueryUserInfoCompleteDelegateHandle = GetUserInt()->OnQueryUserInfoCompleteDelegates->AddWeakLambda(
			this, [OnComplete, this](
				int32 LocalUserNum,
				bool bSucceeded,
				const TArray<FUniqueNetIdRef>& UserIds,
				const FString& ErrorMessage)
			{
				OnQueryUserInfoComplete(LocalUserNum, bSucceeded, UserIds, ErrorMessage, OnComplete);
			});

		if (!GetUserInt()->QueryUserInfo(LocalUserNum, UserIds))
		{
			OnQueryUserInfoComplete(LocalUserNum, false, UserIds, "", OnComplete);
		}
	}
}

void UMatchmakingDSOnlineSession::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	const TArray<const FBaseUserInfo*> UserInfo;
	if (DSRetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete, UserInfo]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	else
	{
		// gather user ids
		TArray<FString> AbUserIds;
		for (const FUniqueNetIdRef& UserId : UserIds)
		{
			const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*UserId);
			const FString AbUserId = AbUniqueNetId->GetAccelByteId();
			if (!AbUserId.IsEmpty())
			{
				AbUserIds.Add(AbUserId);
			}
		}

		AccelByte::FRegistry::User.BulkGetUserInfo(
			AbUserIds,
			THandler<FListBulkUserInfo>::CreateWeakLambda(this, [OnComplete, this](FListBulkUserInfo UserInfo)
			{
				OnDSQueryUserInfoComplete(UserInfo, OnComplete);
			}),
			FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32, const FString&)
			{
				ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete(FListBulkUserInfo(), OnComplete);
				}));
			})
		);
	}
}

bool UMatchmakingDSOnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get Session Info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Can't find player controller with the session's local owner's Unique Id"));
		return false;
	}

	AAccelByteWarsPlayerController* AbPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AbPlayerController)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Player controller is not (derived from) AAccelByteWarsPlayerController"));
		return false;
	}

	// Make sure this is not a P2P session
	if (GetABSessionInt()->IsPlayerP2PHost(GetLocalPlayerUniqueNetId(PlayerController).ToSharedRef().Get(), SessionName)) 
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session is a P2P session"));
		return false;
	}
	
	FString ServerAddress = "";
	GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);

	if (ServerAddress.IsEmpty())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Can't find session's server address"));
		return false;
	}

	if (!bIsInSessionServer)
	{
		AbPlayerController->DelayedClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Already in session's server"));
	}

	return true;
}

void UMatchmakingDSOnlineSession::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	// reset delegate handle
	GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
	OnQueryUserInfoCompleteDelegateHandle.Reset();

	if (bSucceeded)
	{
		// Cache the result.
		CacheUserInfo(LocalUserNum, UserIds);

		// Retrieve the result from cache.
		TArray<FUserOnlineAccountAccelByte*> OnlineUsers;
		RetrieveUserInfoCache(UserIds, OnlineUsers);

		// Only include valid users info only.
		OnlineUsers.RemoveAll([](const FUserOnlineAccountAccelByte* Temp)
		{
			return !Temp || !Temp->GetUserId()->IsValid();
		});

		UE_LOG_MATCHMAKINGDS(Log,
			TEXT("Queried users info: %d, found valid users info: %d"),
			UserIds.Num(), OnlineUsers.Num());

		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

void UMatchmakingDSOnlineSession::OnDSQueryUserInfoComplete(
	const FListBulkUserInfo& UserInfoList,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// reset delegate handle
	OnDSQueryUserInfoCompleteDelegateHandle.Reset();

	if (UserInfoList.Data.IsEmpty())
	{
		OnComplete.ExecuteIfBound(false, {});
	}

	CacheUserInfo(UserInfoList);

	TArray<const FBaseUserInfo*> OnlineUsers;
	for (const FBaseUserInfo& User : UserInfoList.Data)
	{
		OnlineUsers.Add(&User);
	}
	OnComplete.ExecuteIfBound(true, OnlineUsers);
}

void UMatchmakingDSOnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	if (bLeavingSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}

void UMatchmakingDSOnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}

bool UMatchmakingDSOnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
#pragma endregion

#pragma region "Matchmaking Session Essentials"
void UMatchmakingDSOnlineSession::StartMatchmaking(
	const APlayerController* PC,
	const FName& SessionName,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// If the player is already in a session, then leave session first.
	if (GetSession(SessionName))
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("Already in session. Leaving session first."))
		if (OnLeaveSessionForReMatchmakingCompleteDelegateHandle.IsValid())
		{
			GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForReMatchmakingCompleteDelegateHandle);
			OnLeaveSessionForReMatchmakingCompleteDelegateHandle.Reset();
		}

		OnLeaveSessionForReMatchmakingCompleteDelegateHandle = GetOnLeaveSessionCompleteDelegates()->AddUObject(
			this,
			&ThisClass::OnLeaveSessionForReMatchmakingComplete,
			GetLocalUserNumFromPlayerController(PC),
			GameModeType);
		LeaveSession(SessionName);
		return;
	}

	const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
	if (!ensure(PlayerNetId.IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Player UniqueNetId is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}
	
	// Get match pool id based on game mode type
	FString MatchPoolId = MatchPoolIds[GameModeType];

	// if not using AMS, remove suffix -ams (internal purpose)
	if(!UTutorialModuleOnlineUtility::GetIsServerUseAMS())
	{
		MatchPoolId = MatchPoolId.Replace(TEXT("-ams"), TEXT(""));
	}
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(
		SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);

	// Check for DS version override.
	const FString OverriddenDSVersion = UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride();
	if (!OverriddenDSVersion.IsEmpty())
	{
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_CLIENTVERSION, OverriddenDSVersion, EOnlineComparisonOp::Equals);
	}

	// Set local server name for matchmaking request if any.
	// This is useful if you want to try matchmaking using local dedicated server.
	FString ServerName;
	FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
	if (!ServerName.IsEmpty())
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("Requesting local server with name: %s"), *ServerName)
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName, EOnlineComparisonOp::Equals);
	}

	if (!GetSessionInt()->StartMatchmaking(
		USER_ID_TO_MATCHMAKING_USER_ARRAY(PlayerNetId.ToSharedRef()),
		SessionName,
		FOnlineSessionSettings(),
		MatchmakingSearchHandle,
		FOnStartMatchmakingComplete::CreateUObject(this, &ThisClass::OnStartMatchmakingComplete)))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}

void UMatchmakingDSOnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->CurrentMatchmakingSearchHandle.IsValid() &&
		GetABSessionInt()->CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Searching player ID is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!GetSessionInt()->CancelMatchmaking(
		*GetABSessionInt()->CurrentMatchmakingSearchHandle->SearchingPlayerId,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
	}
}

void UMatchmakingDSOnlineSession::OnStartMatchmakingComplete(
	FName SessionName,
	const FOnlineError& ErrorDetails,
	const FSessionMatchmakingResults& Results)
{
	UE_LOG_MATCHMAKINGDS(
		Log,
		TEXT("succeeded: %s | error: (%s) %s"),
		*FString(ErrorDetails.bSucceeded ? "TRUE": "FALSE"),
		*ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString())

	OnStartMatchmakingCompleteDelegates.Broadcast(SessionName, ErrorDetails.bSucceeded);
}

void UMatchmakingDSOnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UMatchmakingDSOnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->CurrentMatchmakingSearchHandle;
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when the MM finished just as we are about to cancel it*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("There is no match result returned."));
		OnMatchmakingCompleteDelegates.Broadcast(SessionName, false);
		return;
	}

	OnMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);

	// Get searching player
	const int32 LocalUserNum =
		GetLocalUserNumFromPlayerController(GetPlayerControllerByUniqueNetId(CurrentMatchmakingSearchHandle->SearchingPlayerId));

	// Join the first session from matchmaking result.
	JoinSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		CurrentMatchmakingSearchHandle->SearchResults[0]);
}

void UMatchmakingDSOnlineSession::OnBackfillProposalReceived(
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// Safety
	if (!ensure(GetABSessionInt().IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	// Accept backfill proposal.
	GetABSessionInt()->AcceptBackfillProposal(
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Proposal,
		false,
		FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [this](bool bSucceeded)
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s To accept backfill."), *FString(bSucceeded ? "TRUE": "FALSE"));
		OnAcceptBackfillProposalCompleteDelegates.Broadcast(bSucceeded);
	}));
}

void UMatchmakingDSOnlineSession::OnLeaveSessionForReMatchmakingComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const EGameModeType GameModeType)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForReMatchmakingCompleteDelegateHandle);

	if (bSucceeded)
	{
		// Retry matchmaking.
		const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
		if (!ensure(PC))
		{
			UE_LOG_MATCHMAKINGDS(Warning, TEXT("PlayerController is null."));
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
			return;
		}

		StartMatchmaking(PC, SessionName, EGameModeNetworkType::DS, GameModeType);
	}
	else
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Is not a game session."));
		OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
	}
}
#pragma endregion 
