// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2POnlineSession.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Interfaces/OnlineUserInterface.h"

void UMatchmakingP2POnlineSession::RegisterOnlineDelegates()
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

	// Matchmaking delegates
	GetSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);
}

void UMatchmakingP2POnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);

	GetSessionInt()->OnMatchmakingCompleteDelegates.RemoveAll(this);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.RemoveAll(this);
}

void UMatchmakingP2POnlineSession::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	Super::OnJoinSessionComplete(SessionName, Result);

	TravelToSession(SessionName);
}

void UMatchmakingP2POnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnLeaveSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
}

#pragma region "Game Session Essentials"
void UMatchmakingP2POnlineSession::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("User interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	// Some data does not exist in cache, query everything
	else
	{
		// Bind delegate
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

bool UMatchmakingP2POnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get Session Info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Can't find player controller with the session's local owner's Unique Id"));
		return false;
	}

	FString ServerAddress = "";

	// If local user is not the P2P host -> connect to host
	if (!GetABSessionInt()->IsPlayerP2PHost(GetLocalPlayerUniqueNetId(PlayerController).ToSharedRef().Get(), SessionName)) 
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Is not P2P host, travelling to host"));
		GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);
		if (ServerAddress.IsEmpty())
		{
			UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Can't find session's server address"));
			return false;
		}
	}
	else
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Is P2P host, travelling as listen server"));
		ServerAddress = "MainMenu?listen";
	}

	if (!bIsInSessionServer)
	{
		PlayerController->ClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Already in session's server"));
	}

	return true;
}

void UMatchmakingP2POnlineSession::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	// reset delegate handle
	GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
	OnQueryUserInfoCompleteDelegateHandle.Reset();

	if (bSucceeded)
	{
		TArray<FUserOnlineAccountAccelByte*> OnlineUsers;
		if (!RetrieveUserInfoCache(UserIds, OnlineUsers))
		{
			CacheUserInfo(LocalUserNum, UserIds);

			/**
			 * Only include valid users info, in the case of invalid user ids, doesn't exist in backend,
			 * their data simply would not exist in the OnComplete delegate.
			 * Asuumes data does not exist in backend if users info is not in the OnComplete's parameter.
			 */
			for (const FUniqueNetIdRef& UserId : UserIds)
			{
				TSharedPtr<FOnlineUser> OnlineUserPtr = GetUserInt()->GetUserInfo(LocalUserNum, UserId.Get());
				if (OnlineUserPtr.IsValid())
				{
					TSharedPtr<FUserOnlineAccountAccelByte> AbUserPtr = StaticCastSharedPtr<
						FUserOnlineAccountAccelByte>(OnlineUserPtr);
					OnlineUsers.AddUnique(AbUserPtr.Get());
				}
			}
		}
		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

void UMatchmakingP2POnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	if (bLeaveSessionRunning)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}

void UMatchmakingP2POnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}

bool UMatchmakingP2POnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
#pragma endregion

#pragma region "Matchmaking Session Essentials"
void UMatchmakingP2POnlineSession::StartMatchmaking(
	const APlayerController* PC,
	const FName& SessionName,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// If the player is already in a session, then leave session first.
	if (GetSession(SessionName))
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Already in session. Leaving session first."))
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
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Player UniqueNetId is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// Get match pool id based on game mode type
	const FString MatchPoolId = MatchPoolIds[GameModeType];
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(
		SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);

	if (!GetSessionInt()->StartMatchmaking(
		USER_ID_TO_MATCHMAKING_USER_ARRAY(PlayerNetId.ToSharedRef()),
		SessionName,
		FOnlineSessionSettings(),
		MatchmakingSearchHandle,
		FOnStartMatchmakingComplete::CreateUObject(this, &ThisClass::OnStartMatchmakingComplete)))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}

void UMatchmakingP2POnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->CurrentMatchmakingSearchHandle.IsValid() &&
		GetABSessionInt()->CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Searching player ID is not valid."));
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
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
	}
}

void UMatchmakingP2POnlineSession::OnStartMatchmakingComplete(
	FName SessionName,
	const FOnlineError& ErrorDetails,
	const FSessionMatchmakingResults& Results)
{
	UE_LOG_MATCHMAKINGP2P(
		Log,
		TEXT("succeeded: %s | error: (%s) %s"),
		*FString(ErrorDetails.bSucceeded ? "TRUE": "FALSE"),
		*ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString())

	OnStartMatchmakingCompleteDelegates.Broadcast(SessionName, ErrorDetails.bSucceeded);
}

void UMatchmakingP2POnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UMatchmakingP2POnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->CurrentMatchmakingSearchHandle;
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when the MM finished just as we are about to cancel it*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("There is no match result returned."));
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

void UMatchmakingP2POnlineSession::OnBackfillProposalReceived(
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// Safety
	if (!ensure(GetABSessionInt().IsValid()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	// Accept backfill proposal.
	GetABSessionInt()->AcceptBackfillProposal(
		NAME_GameSession,
		Proposal,
		false,
		FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [this](bool bSucceeded)
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s To accept backfill."), *FString(bSucceeded ? "TRUE": "FALSE"));
		OnAcceptBackfillProposalCompleteDelegates.Broadcast(bSucceeded);
	}));
}

void UMatchmakingP2POnlineSession::OnLeaveSessionForReMatchmakingComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const EGameModeType GameModeType)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForReMatchmakingCompleteDelegateHandle);

	if (bSucceeded)
	{
		// Retry matchmaking.
		const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
		if (!ensure(PC))
		{
			UE_LOG_MATCHMAKINGP2P(Warning, TEXT("PlayerController is null."));
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
			return;
		}

		StartMatchmaking(PC, SessionName, EGameModeNetworkType::P2P, GameModeType);
	}
	else
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Is not a game session."));
		OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
	}
}
#pragma endregion 
