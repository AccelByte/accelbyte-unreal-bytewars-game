// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingDSOnlineSession.h"

#include "MatchmakingDSLog.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-RegisterOnlineDelegates
// @@@MULTISNIP BindMatchmakingDelegate {"selectedLines": ["1-2", "19", "22"]}
// @@@MULTISNIP BindCancelMatchmakingDelegate {"selectedLines": ["1-2", "20", "22"]}
// @@@MULTISNIP BindSessionServerDelegate {"selectedLines": ["1-2", "6-7", "22"]}
// @@@MULTISNIP BindBackfillDelegate {"selectedLines": ["1-2", "21", "22"]}
// @@@MULTISNIP BindLeaveSessionDelegate {"selectedLines": ["1-2", "9-16", "22"]}
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
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-ClearOnlineDelegates
// @@@MULTISNIP UnbindMatchmakingDelegate {"selectedLines": ["1-2", "12", "15"]}
// @@@MULTISNIP UnbindCancelMatchmakingDelegate {"selectedLines": ["1-2", "13", "15"]}
// @@@MULTISNIP UnbindSessionServerDelegate {"selectedLines": ["1-2", "5-6", "15"]}
// @@@MULTISNIP UnbindBackfillDelegate {"selectedLines": ["1-2", "14", "15"]}
// @@@MULTISNIP UnbindLeaveSessionDelegate {"selectedLines": ["1-2", "8-10", "15"]}
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
// @@@SNIPEND

#pragma region "Game Session Essentials"
// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-DSQueryUserInfo
void UMatchmakingDSOnlineSession::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	const TArray<const FBaseUserInfo*> UserInfo;
	if (DSRetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHMAKINGDS(Log, TEXT("Cache found"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete, UserInfo]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	else
	{
		// Gather user IDs.
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
				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete(FListBulkUserInfo(), OnComplete);
				}));
			})
		);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-TravelToSession
bool UMatchmakingDSOnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get session info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("The session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("The session info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("The session info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Can't find player controller with the session's local owner's unique ID"));
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
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("The session is a P2P session"));
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
// @@@SNIPEND

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

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnSessionServerUpdateReceived
void UMatchmakingDSOnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	if (bLeavingSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("called but leave session is currently running. Canceling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnSessionServerErrorReceived
void UMatchmakingDSOnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}
// @@@SNIPEND

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
// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-StartMatchmaking
void UMatchmakingDSOnlineSession::StartMatchmaking(
	const APlayerController* PC,
	const FName& SessionName,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}
	
	// Get match pool ID based on game mode type
	FString MatchPoolId = MatchPoolIds[GameModeType];
	const FString GameModeCode = TargetGameModeMap[MatchPoolId];

	// AMS is the default multiplayer server on AGS. If the game runs on legacy AGS Armada, remove the -ams suffix.
	if(!UTutorialModuleOnlineUtility::GetIsServerUseAMS())
	{
		MatchPoolId = MatchPoolId.Replace(TEXT("-ams"), TEXT(""));
	}
	
	// Override match pool ID if applicable.
	if (!UTutorialModuleOnlineUtility::GetMatchPoolDSOverride().IsEmpty())
	{
		MatchPoolId = UTutorialModuleOnlineUtility::GetMatchPoolDSOverride();
	}

	// Set up matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);
	MatchmakingSearchHandle->QuerySettings.Set(GAMESETUP_GameModeCode, GameModeCode, EOnlineComparisonOp::Equals);

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

	// Include region preferences into matchmaking setting.
	if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
		this,
		true))
	{
		if (!ModuleDataAsset->IsStarterModeActive())
		{
			UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
			ensure(GameInstance);

			URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
			if(RegionPreferencesSubsystem != nullptr)
			{
				TArray<FString> EnabledRegion = RegionPreferencesSubsystem->GetEnabledRegion();
				if(!EnabledRegion.IsEmpty())
				{
					FOnlineSearchSettingsAccelByte::Set(MatchmakingSearchHandle->QuerySettings, SETTING_GAMESESSION_REQUESTEDREGIONS, RegionPreferencesSubsystem->GetEnabledRegion(), EOnlineComparisonOp::In);
				}
			}
		}
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-CancelMatchmaking
void UMatchmakingDSOnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->GetCurrentMatchmakingSearchHandle().IsValid() &&
		GetABSessionInt()->GetCurrentMatchmakingSearchHandle()->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Searching player ID is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!GetSessionInt()->CancelMatchmaking(
		*GetABSessionInt()->GetCurrentMatchmakingSearchHandle()->GetSearchingPlayerId(),
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnStartMatchmakingComplete
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
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnCancelMatchmakingComplete
void UMatchmakingDSOnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnMatchmakingComplete
void UMatchmakingDSOnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->GetCurrentMatchmakingSearchHandle();
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when matchmaking finishes right as it’s about to be canceled.*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("There is no match result returned."));
		OnMatchmakingCompleteDelegates.Broadcast(SessionName, false);
		return;
	}

	OnMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnBackfillProposalReceived
void UMatchmakingDSOnlineSession::OnBackfillProposalReceived(
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
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
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.cpp-OnLeaveSessionForReMatchmakingComplete
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
// @@@SNIPEND
#pragma endregion 
