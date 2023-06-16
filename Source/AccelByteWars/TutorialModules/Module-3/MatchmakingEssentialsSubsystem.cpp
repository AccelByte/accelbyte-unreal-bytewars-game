// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"


void UMatchmakingEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Online Subsystem and make sure it's valid.
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem)) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte Session Interface and make sure it's valid.
	SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid())) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	// Bind delegates to game events.
	AAccelByteWarsGameMode::OnAddOnlineMemberDelegate.AddUObject(this, &ThisClass::SetTeamMemberAccelByteInformation);
	UMatchLobbyWidget::OnQuitLobbyDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
	UPauseWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
	UGameOverWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);

	if (IsRunningDedicatedServer())
	{
		OnServerReceivedSessionDelegateHandle = SessionInterface->AddOnServerReceivedSessionDelegate_Handle(FOnServerReceivedSessionDelegate::CreateUObject(this, &ThisClass::OnServerReceivedSession));
	}

	// Bind delegates that needed for the tutorial module.
	// Intended for the reader of the tutorial module to follow along.
	BindDelegates();
}

void UMatchmakingEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// Unbind delegates from game events.
	AAccelByteWarsGameMode::OnAddOnlineMemberDelegate.Clear();
	UMatchLobbyWidget::OnQuitLobbyDelegate.Clear();
	UPauseWidget::OnQuitGameDelegate.Clear();
	UGameOverWidget::OnQuitGameDelegate.Clear();

	if (!SessionInterface.IsValid())
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	if (IsRunningDedicatedServer())
	{
		SessionInterface->ClearOnServerReceivedSessionDelegate_Handle(OnServerReceivedSessionDelegateHandle);
	}

	// Unbind delegates that used for the tutorial module.
	// Intended for the reader of the tutorial module to follow along.
	UnbindDelegates();
}

bool UMatchmakingEssentialsSubsystem::IsGameSessionValid(FName SessionName)
{
	return (SessionName == NAME_GameSession);
}

FUniqueNetIdPtr UMatchmakingEssentialsSubsystem::GetPlayerUniqueNetId(APlayerController* PC) const
{
	if (!ensure(PC)) 
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

void UMatchmakingEssentialsSubsystem::OnServerReceivedSession(FName SessionName)
{
	if (!IsGameSessionValid(SessionName))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Server cannot handle received game session. Game Session is invalid."));
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Server cannot handle received game session. Session in null."));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Server cannot handle received game session. Session Info is not valid."));
		return;
	}

	AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(GetWorld()->GetGameState());
	if (!ensure(GameState)) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Server cannot handle received game session. Game State is null."));
		return;
	}

	// Set server's game mode.
	const FString GameMode = SessionInfo->GetBackendSessionData()->Configuration.Name.Replace(TEXT("unreal-"), TEXT(""));
	if (!GameMode.IsEmpty())
	{
		GameState->AssignGameMode(GameMode.ToUpper());
	}
}

void UMatchmakingEssentialsSubsystem::SetTeamMemberAccelByteInformation(APlayerController* PC, TDelegate<void(bool /*bIsSuccessful*/)> OnComplete)
{
	const FUniqueNetIdPtr PlayerNetId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
	if (!PlayerNetId.IsValid())
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player Net Id is invalid. Cannot get player AccelByte's information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	const FUniqueNetIdAccelByteUserPtr AccelBytePlayerNetId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerNetId);
	if (!ensure(AccelBytePlayerNetId.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player's AccelByte Net Id us not valid. Cannot get player AccelByte's information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = StaticCast<AAccelByteWarsPlayerState*>(PC->PlayerState);
	if (!PlayerState)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Player State is null. Cannot get player's AccelByte information."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	FRegistry::User.BulkGetUserInfo(
		{ AccelBytePlayerNetId->GetAccelByteId() },
		THandler<FListBulkUserInfo>::CreateWeakLambda(this, [PlayerState, OnComplete](const FListBulkUserInfo& Result)
		{
			const FBaseUserInfo& PlayerInfo = Result.Data[0];
			PlayerState->SetPlayerName(PlayerInfo.DisplayName);
			PlayerState->AvatarURL = PlayerInfo.AvatarUrl;

			OnComplete.ExecuteIfBound(true);
		}),
		FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
		{
			OnComplete.ExecuteIfBound(false);
		})
	);
}


#pragma region Module.3 General Function Definitions
void UMatchmakingEssentialsSubsystem::BindDelegates()
{
	// Bind delegates to game events.
	AAccelByteWarsGameSession::OnRegisterServerDelegate.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameMode::OnGetTeamIdFromSessionDelegate.AddUObject(this, &ThisClass::GetTeamIdFromSession);
	AAccelByteWarsGameSession::OnUnregisterServerDelegate.AddUObject(this, &ThisClass::UnregisterServer);

	// Listen to backfill proposal. Run it only on game server.
	if (IsRunningDedicatedServer())
	{
		BackfillProposalReceivedDelegateHandle = SessionInterface->AddOnBackfillProposalReceivedDelegate_Handle(FOnBackfillProposalReceivedDelegate::CreateUObject(this, &ThisClass::OnBackfillProposalReceived));
	}
}

void UMatchmakingEssentialsSubsystem::UnbindDelegates()
{
	// Unbind delegates from game events.
	AAccelByteWarsGameSession::OnRegisterServerDelegate.Clear();
	AAccelByteWarsGameMode::OnGetTeamIdFromSessionDelegate.Clear();
	AAccelByteWarsGameSession::OnUnregisterServerDelegate.Clear();

	// Unbind backfill proposal listener. Run it only on game server.
	if (IsRunningDedicatedServer())
	{
		SessionInterface->ClearOnBackfillProposalReceivedDelegate_Handle(BackfillProposalReceivedDelegateHandle);
	}
}

void UMatchmakingEssentialsSubsystem::OnQuitGameButtonsClicked(APlayerController* PC)
{
	LeaveSession(PC);
}
#pragma endregion


#pragma region Module.3a Function Definitions
void UMatchmakingEssentialsSubsystem::StartMatchmaking(APlayerController* PC, const FString& MatchPool, const FOnMatchmakingStateChangedDelegate& OnMatchmaking)
{
	// Save the matchmaking callback delegate. It will be used to inform the matchmaking states back to the player.
	OnMatchmakingHandle = OnMatchmaking;

	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::StartMatchmaking, FAILED_MESSAGE_NONE);

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot start matchmaking. Session Interface is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	// If the player is already in a session, then destroy it first.
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		const FOnDestroySessionCompleteDelegate OnDestroyToRematchmakingCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroyToRematchmakingComplete, PC, MatchPool);
		SessionInterface->DestroySession(NAME_GameSession, OnDestroyToRematchmakingCompleteDelegate);
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = GetPlayerUniqueNetId(PC);
	if (!ensure(LocalPlayerId.IsValid())) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot start matchmaking. LocalPlayer NetId is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(SETTING_SESSION_MATCHPOOL, MatchPool, EOnlineComparisonOp::Equals);

	// Set local server name for matchmaking request if any.
	// This is useful if you want to try matchmaking using local dedicated server.
	FString ServerName;
	FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
	if (!ServerName.IsEmpty())
	{
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName, EOnlineComparisonOp::Equals);
	}

	// Bind on-matchmaking complete delegate.
	MatchmakingCompleteDelegateHandle = SessionInterface->AddOnMatchmakingCompleteDelegate_Handle(FOnMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnMatchmakingComplete, PC));

	// Bind on-start matchmaking delegate and then start the matchmaking process.
	const FOnStartMatchmakingComplete OnStartMatchmakingCompleteDelegate = FOnStartMatchmakingComplete::CreateUObject(this, &ThisClass::OnStartMatchmakingComplete);
	if (SessionInterface->StartMatchmaking(USER_ID_TO_MATCHMAKING_USER_ARRAY(LocalPlayerId.ToSharedRef()), NAME_GameSession, FOnlineSessionSettings(), MatchmakingSearchHandle, OnStartMatchmakingCompleteDelegate))
	{
		// If success, save the matchmaking search results.
		CurrentMatchmakingSearchHandle = MatchmakingSearchHandle;
	}
	else
	{
		// Failed to start matchmaking.
		SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
	}
}

void UMatchmakingEssentialsSubsystem::OnDestroyToRematchmakingComplete(FName SessionName, bool bWasSuccessful, APlayerController* PC, const FString LastMatchPool)
{
	if (IsGameSessionValid(SessionName) && bWasSuccessful)
	{
		// Retry matchmaking.
		if (!ensure(PC))
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to rematchmaking. Player Controller is null."));
			OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
			return;
		}

		StartMatchmaking(PC, LastMatchPool, OnMatchmakingHandle);
	}
	else
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
	}
}

void UMatchmakingEssentialsSubsystem::OnStartMatchmakingComplete(FName SessionName, const FOnlineError& ErrorDetails, const FSessionMatchmakingResults& Results)
{
	if (!IsGameSessionValid(SessionName) || !ErrorDetails.bSucceeded)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Start matchmaking failed %s: %s"), *ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString());
		CurrentMatchmakingSearchHandle.Reset();
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);

		// Unbind on-matchmaking completed delegate.
		if (SessionInterface.IsValid()) 
		{
			SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
		}
	}
	else
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindingMatch, FAILED_MESSAGE_NONE);
	}
}

void UMatchmakingEssentialsSubsystem::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("OnMatchmakingComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	// Unbind on-matchmaking completed delegate.
	if (SessionInterface.IsValid()) 
	{
		SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
	}

	if (!bWasSuccessful || !ensure(CurrentMatchmakingSearchHandle.IsValid()) || !ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Matchmaking is failed or there is no match result returned."));
		CurrentMatchmakingSearchHandle.Reset();
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	// Join the first session from matchmaking result.
	JoinSession(CurrentMatchmakingSearchHandle->SearchResults[0], PC);
	CurrentMatchmakingSearchHandle.Reset();
}

void UMatchmakingEssentialsSubsystem::CancelMatchmaking(APlayerController* PC)
{
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot cancel matchmaking. Session Interface is not valid."));
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!ensure(LocalPlayer)) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot cancel matchmaking. LocalPlayer is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!ensure(LocalPlayerId.IsValid())) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot cancel matchmaking. LocalPlayer NetId is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	// Unbind on-matchmaking complete delegate.
	SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
	CurrentMatchmakingSearchHandle.Reset();

	// Bind on-cancel matchmaking completed and start matchmaking cancelation process.
	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::CancelingMatch, FAILED_MESSAGE_NONE);
	CancelMatchmakingCompleteDelegateHandle = SessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelMatchmakingComplete));
	SessionInterface->CancelMatchmaking(LocalPlayerId.ToSharedRef().Get(), NAME_GameSession);
}

void UMatchmakingEssentialsSubsystem::OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	if (!IsGameSessionValid(SessionName))
	{
		return;
	}

	// Unbind on-cancel matchmaking completed delegate.
	if (SessionInterface.IsValid()) 
	{
		SessionInterface->ClearOnCancelMatchmakingCompleteDelegate_Handle(CancelMatchmakingCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Success to cancel matchmaking."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::Default, FAILED_MESSAGE_NONE);
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to cancel matchmaking."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_CANCEL_MATCH);
	}
}
#pragma endregion 


#pragma region Module.3b Function Definitions
void UMatchmakingEssentialsSubsystem::RegisterServer(FName SessionName)
{
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot register server. Session Interface is null."));
		return;
	}

	SessionInterface->RegisterServer(SessionName);
}

void UMatchmakingEssentialsSubsystem::GetTeamIdFromSession(FName SessionName, const FUniqueNetIdRepl& UniqueNetId, int32& OutTeamId)
{
	OutTeamId = INDEX_NONE;

	if (!IsGameSessionValid(SessionName)) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Game session is not valid."));
		return;
	}

	if (!UniqueNetId.IsValid()) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Player's UniqueNetId is null."));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr PlayerNetId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(UniqueNetId.GetUniqueNetId());
	if (!ensure(PlayerNetId))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Player's AccelByte UniqueNetId is not valid."));
		return;
	}

	// Get Session Info. It is used to get team assignment from backend.
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Session Interface is null."));
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Session in null."));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Session Info is not valid."));
		return;
	}

	// Try assign player to team based on Session Info.
	int32 TeamIndex = 0;
	TArray<FAccelByteModelsV2GameSessionTeam> Teams = SessionInfo->GetTeamAssignments();
	for (FAccelByteModelsV2GameSessionTeam Team : Teams)
	{
		for (FString MemberId : Team.UserIDs)
		{
			if (PlayerNetId->GetAccelByteId() == MemberId)
			{
				OutTeamId = TeamIndex;
				return;
			}
		}

		TeamIndex++;
	}
}

void UMatchmakingEssentialsSubsystem::UnregisterServer(FName SessionName)
{
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot unregister server. Session Interface is null."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	if (!ensure(SessionInterface->GetNamedSession(SessionName)))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot unregister server. Session is null."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	SessionInterface->UnregisterServer(SessionName, FOnUnregisterServerComplete::CreateWeakLambda(this, [](bool bWasSuccessful)
	{
		if (bWasSuccessful)
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Success to unregister server from AMS."));
			FPlatformMisc::RequestExit(false);
		}
		else
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to unregister server from AMS."));
			FPlatformMisc::RequestExit(false);
		}
	}));
}

void UMatchmakingEssentialsSubsystem::OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot handle backfill proposal. Session Interface is not valid."));
		return;
	}

	// Accept backfill proposal.
	SessionInterface->AcceptBackfillProposal(NAME_GameSession, Proposal, false, FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [this](bool bWasSuccessful)
	{
		if (bWasSuccessful)
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Success to accept backfill."));
		}
		else
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to accept backfill."));
			OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		}
	}));
}
#pragma endregion


#pragma region Module.3c Function Definitions
void UMatchmakingEssentialsSubsystem::JoinSession(const FOnlineSessionSearchResult& Session, APlayerController* PC)
{
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot join game session. Session Interface is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_JOIN_MATCH);
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = GetPlayerUniqueNetId(PC);
	if (!ensure(LocalPlayerId.IsValid())) 
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot join game session. LocalPlayer NetId is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_MATCH);
		return;
	}

	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::JoiningMatch, FAILED_MESSAGE_NONE);

	// Bind on-join session completed and start join session process.
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete, PC));
	SessionInterface->JoinSession(LocalPlayerId.ToSharedRef().Get(), NAME_GameSession, Session);
}

void UMatchmakingEssentialsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_JOIN_MATCH);
		return;
	}

	// Unbind on-join session completed delegate.
	if (SessionInterface.IsValid()) 
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to join the session."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_JOIN_MATCH);
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Log, TEXT("Success to join session. Session Name: %s"), *SessionName.ToString());

		// When the player success to join the game session, it is possible that the game sesison already has server info.
		// Therefore, try to travel the client to the server using the information available in the current game session first.
		// If it fails, then bind a delegate to listen when the server is ready to travel.
		if (!TravelClient(SessionName, PC))
		{
			SessionServerUpdateDelegateHandle = SessionInterface->AddOnSessionServerUpdateDelegate_Handle(FOnSessionServerUpdateDelegate::CreateUObject(this, &ThisClass::OnSessionServerUpdate, PC));
			SessionServerErrorDelegateHandle = SessionInterface->AddOnSessionServerErrorDelegate_Handle(FOnSessionServerErrorDelegate::CreateUObject(this, &ThisClass::OnSessionServerError));
		}
		else 
		{
			// The game client success to travel to the server. The whole matchmaking process is complete.
			OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::MatchFound, FAILED_MESSAGE_NONE);
		}
	}
}

bool UMatchmakingEssentialsSubsystem::TravelClient(FName SessionName, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		return false;
	}

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session Interface is not valid."));
		return false;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session is not null."));
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session Info is not valid."));
		return false;
	}

	if (!ensure(PC))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. PlayerController is not valid."));
		return false;
	}

	// Travel the client to the server via URL.
	FString TravelUrl{};
	if (SessionInterface->GetResolvedConnectString(SessionName, TravelUrl) && !TravelUrl.IsEmpty())
	{
		PC->ClientTravel(TravelUrl, TRAVEL_Absolute);
		return true;
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Travel URL is not valid."));
		return false;
	}
}

void UMatchmakingEssentialsSubsystem::OnSessionServerUpdate(FName SessionName, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		return;
	}

	if (SessionInterface.IsValid()) 
	{
		SessionInterface->ClearOnSessionServerUpdateDelegate_Handle(SessionServerUpdateDelegateHandle);
	}

	// The server is ready. Try to travel the game clients to the server.
	if (TravelClient(SessionName, PC)) 
	{
		// The game client success to travel to the server. The whole matchmaking process is complete.
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::MatchFound, FAILED_MESSAGE_NONE);
	}
	else 
	{
		// The game client failed to travel to the server.
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_SERVER);
	}
}

void UMatchmakingEssentialsSubsystem::OnSessionServerError(FName SessionName, const FString& ErrorMessage)
{
	if (!IsGameSessionValid(SessionName))
	{
		return;
	}

	UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to travel to the game server. Error: %s"), *ErrorMessage);
	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed, FAILED_FIND_SERVER);

	if (SessionInterface.IsValid()) 
	{
		SessionInterface->ClearOnSessionServerErrorDelegate_Handle(SessionServerErrorDelegateHandle);
	}
}

void UMatchmakingEssentialsSubsystem::LeaveSession(APlayerController* PC)
{
	// If running on local gameplay, player doesn't need to leave game session.
	if (PC->GetNetMode() == ENetMode::NM_Standalone)
	{
		return;
	}

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot leave game session. Session Interface is not valid."));
		return;
	}

	SessionInterface->DestroySession(
		NAME_GameSession,
		FOnDestroySessionCompleteDelegate::CreateWeakLambda(this, [](FName SessionName, bool bWasSuccessful)
		{
			if (bWasSuccessful)
			{
				UE_LOG_MATCHMAKING_ESSENTIALS(Log, TEXT("Success to leave game session."));
			}
			else
			{
				UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to leave game session."));
			}
		})
	);
}
#pragma endregion