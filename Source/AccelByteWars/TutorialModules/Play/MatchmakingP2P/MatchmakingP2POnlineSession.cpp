// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2POnlineSession.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineUserInterface.h"

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-RegisterOnlineDelegates
// @@@MULTISNIP BindMatchmakingDelegate {"selectedLines": ["1-2", "19", "22"]}
// @@@MULTISNIP BindCancelMatchmakingDelegate {"selectedLines": ["1-2", "20", "22"]}
// @@@MULTISNIP BindSessionServerDelegate {"selectedLines": ["1-2", "6-7", "22"]}
// @@@MULTISNIP BindBackfillDelegate {"selectedLines": ["1-2", "21", "22"]}
// @@@MULTISNIP BindLeaveSessionDelegate {"selectedLines": ["1-2", "9-16", "22"]}
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
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Matchmaking delegates
	GetSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-ClearOnlineDelegates
// @@@MULTISNIP UnbindMatchmakingDelegate {"selectedLines": ["1-2", "12", "15"]}
// @@@MULTISNIP UnbindCancelMatchmakingDelegate {"selectedLines": ["1-2", "13", "15"]}
// @@@MULTISNIP UnbindSessionServerDelegate {"selectedLines": ["1-2", "5-6", "15"]}
// @@@MULTISNIP UnbindBackfillDelegate {"selectedLines": ["1-2", "14", "15"]}
// @@@MULTISNIP UnbindLeaveSessionDelegate {"selectedLines": ["1-2", "8-10", "15"]}
void UMatchmakingP2POnlineSession::ClearOnlineDelegates()
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

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnJoinSessionComplete
void UMatchmakingP2POnlineSession::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	Super::OnJoinSessionComplete(SessionName, Result);

	TravelToSession(SessionName);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnLeaveSessionComplete
void UMatchmakingP2POnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnLeaveSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
}
// @@@SNIPEND

#pragma region "Game Session Essentials"
// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-TravelToSession
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

	AAccelByteWarsPlayerController* AbPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AbPlayerController)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Player controller is not (derived from) AAccelByteWarsPlayerController"));
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
		AbPlayerController->DelayedClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Already in session's server"));
	}

	return true;
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnSessionServerUpdateReceived
void UMatchmakingP2POnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	if (bLeavingSession)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnSessionServerErrorReceived
void UMatchmakingP2POnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}
// @@@SNIPEND

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
// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-StartMatchmaking
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// Get match pool id based on game mode type
	FString MatchPoolId = MatchPoolIds[GameModeType];
	const FString GameModeCode = TargetGameModeMap[MatchPoolId];

	// Override match pool id if applicable.
	if (!UTutorialModuleOnlineUtility::GetMatchPoolP2POverride().IsEmpty())
	{
		MatchPoolId = UTutorialModuleOnlineUtility::GetMatchPoolP2POverride();
	}

	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);
	MatchmakingSearchHandle->QuerySettings.Set(GAMESETUP_GameModeCode, GameModeCode, EOnlineComparisonOp::Equals);

	if (!GetSessionInt()->StartMatchmaking(
		USER_ID_TO_MATCHMAKING_USER_ARRAY(PlayerNetId.ToSharedRef()),
		SessionName,
		FOnlineSessionSettings(),
		MatchmakingSearchHandle,
		FOnStartMatchmakingComplete::CreateUObject(this, &ThisClass::OnStartMatchmakingComplete)))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-CancelMatchmaking
void UMatchmakingP2POnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->GetCurrentMatchmakingSearchHandle().IsValid() &&
		GetABSessionInt()->GetCurrentMatchmakingSearchHandle()->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Searching player ID is not valid."));
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
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnStartMatchmakingComplete
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
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnCancelMatchmakingComplete
void UMatchmakingP2POnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnMatchmakingComplete
void UMatchmakingP2POnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->GetCurrentMatchmakingSearchHandle();
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when the MM finished just as we are about to cancel it*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("There is no match result returned."));
		OnMatchmakingCompleteDelegates.Broadcast(SessionName, false);
		return;
	}

	OnMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnBackfillProposalReceived
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
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Proposal,
		false,
		FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [this](bool bSucceeded)
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s To accept backfill."), *FString(bSucceeded ? "TRUE": "FALSE"));
		OnAcceptBackfillProposalCompleteDelegates.Broadcast(bSucceeded);
	}));
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.cpp-OnLeaveSessionForReMatchmakingComplete
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
// @@@SNIPEND
#pragma endregion 
