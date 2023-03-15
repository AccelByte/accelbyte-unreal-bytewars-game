// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-3/MatchmakingEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

#include "Core/AssetManager/AssetManagementSubsystem.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "Core/GameModes/AccelByteWarsGameModeBase.h"
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

	// Bind delegates to game events if the matchmaking module is active.
	if (UAssetManagementSubsystem* AssetManagement = GetGameInstance()->GetSubsystem<UAssetManagementSubsystem>()) 
	{
		UAccelByteWarsDataAsset* DataAsset = AssetManagement->GetByteWarsAssetManager()->GetAssetFromCache(FPrimaryAssetId{ "TutorialModule:MATCHMAKINGESSENTIALS" });
		if (!DataAsset) 
		{
			return;
		}

		UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(DataAsset);
		if (!TutorialModule || !TutorialModule->bIsActive) 
		{
			return;
		}

		AAccelByteWarsGameModeBase::OnAddOnlineMemberDelegate.AddUObject(this, &ThisClass::SetTeamMemberAccelByteInformation);
		UMatchLobbyWidget::OnQuitLobbyDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
		UPauseWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);
		UGameOverWidget::OnQuitGameDelegate.AddUObject(this, &ThisClass::OnQuitGameButtonsClicked);

		BindDelegates();
	}
}

void UMatchmakingEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameModeBase::OnAddOnlineMemberDelegate.Clear();
	UMatchLobbyWidget::OnQuitLobbyDelegate.Clear();
	UPauseWidget::OnQuitGameDelegate.Clear();
	UGameOverWidget::OnQuitGameDelegate.Clear();

	UnbindDelegates();
}

void UMatchmakingEssentialsSubsystem::BindDelegates()
{
	// Bind delegates to game events.
	AAccelByteWarsGameSession::OnRegisterServerDelegate.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameModeBase::OnGetTeamIdFromSessionDelegate.AddUObject(this, &ThisClass::GetTeamIdFromSession);
	AAccelByteWarsGameSession::OnUnregisterServerDelegate.AddUObject(this, &ThisClass::UnregisterServer);

	// Listen to backfill proposal. Run it only on game server.
	if (IsRunningDedicatedServer())
	{
		SessionInterface->AddOnBackfillProposalReceivedDelegate_Handle(FOnBackfillProposalReceivedDelegate::CreateUObject(this, &ThisClass::OnBackfillProposalReceived));
	}
}

void UMatchmakingEssentialsSubsystem::UnbindDelegates()
{
	// Unbind delegates from game events.
	AAccelByteWarsGameSession::OnRegisterServerDelegate.Clear();
	AAccelByteWarsGameModeBase::OnGetTeamIdFromSessionDelegate.Clear();
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

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PC->PlayerState);
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
			const FBaseUserInfo PlayerInfo = Result.Data[0];
			if (!PlayerInfo.Username.IsEmpty()) 
			{
				PlayerState->SetPlayerName(PlayerInfo.Username);
			}

			PlayerState->AvatarURL = PlayerInfo.AvatarUrl;

			OnComplete.ExecuteIfBound(true);
		}),
		FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
		{
			OnComplete.ExecuteIfBound(false);
		})
	);
}


#pragma region Module.3a Function Definitions
void UMatchmakingEssentialsSubsystem::StartMatchmaking(APlayerController* PC, const FString& MatchPool, const FOnMatchmakingStateChangedDelegate& OnMatchmaking)
{
	// Save the matchmaking callback delegate. It will be used to inform the matchmaking states back to the player.
	OnMatchmakingHandle = OnMatchmaking;

	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindingMatch);

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot start matchmaking. Session Interface is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
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
	ensure(LocalPlayerId.IsValid());

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
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
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
			OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
			return;
		}

		StartMatchmaking(PC, LastMatchPool, OnMatchmakingHandle);
	}
	else
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
	}
}

void UMatchmakingEssentialsSubsystem::OnStartMatchmakingComplete(FName SessionName, const FOnlineError& ErrorDetails, const FSessionMatchmakingResults& Results)
{
	if (!IsGameSessionValid(SessionName) || !ErrorDetails.bSucceeded)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Start matchmaking failed %s: %s"), *ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString());
		CurrentMatchmakingSearchHandle.Reset();
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);

		// Unbind on-matchmaking completed delegate.
		ensure(SessionInterface.IsValid());
		SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
	}
}

void UMatchmakingEssentialsSubsystem::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
		return;
	}

	UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("OnMatchmakingComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	// Unbind on-matchmaking completed delegate.
	ensure(SessionInterface.IsValid());
	SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);

	if (!bWasSuccessful || !ensure(CurrentMatchmakingSearchHandle.IsValid()) || !ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Matchmaking is failed or there is no match result returned."));
		CurrentMatchmakingSearchHandle.Reset();
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
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
	ensure(LocalPlayer);

	const FUniqueNetIdPtr LocalPlayerId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	ensure(LocalPlayerId.IsValid());

	// Unbind on-matchmaking complete delegate.
	SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(MatchmakingCompleteDelegateHandle);
	CurrentMatchmakingSearchHandle.Reset();

	// Bind on-cancel matchmaking completed and start matchmaking cancelation process.
	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::CancelingMatch);
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
	ensure(SessionInterface.IsValid());
	SessionInterface->ClearOnCancelMatchmakingCompleteDelegate_Handle(CancelMatchmakingCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Success to cancel matchmaking."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::Default);
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to cancel matchmaking."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
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

void UMatchmakingEssentialsSubsystem::GetTeamIdFromSession(APlayerController* PC, int32& OutTeamId)
{
	OutTeamId = INDEX_NONE;

	// Ensure the player is valid.
	if (!ensure(PC))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. PlayerController is null."));
		return;
	}

	const FUniqueNetIdPtr RawNetId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
	if (!ensure(RawNetId.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot get team assignment. Player's UniqueNetId is null."));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr PlayerNetId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(RawNetId);
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

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
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
	SessionInterface->AcceptBackfillProposal(NAME_GameSession, Proposal, false, FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [](bool bWasSuccessful)
	{
		if (bWasSuccessful)
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Success to accept backfill."));
		}
		else
		{
			UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to accept backfill."));
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
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = GetPlayerUniqueNetId(PC);
	ensure(LocalPlayerId.IsValid());

	OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::JoiningMatch);

	// Bind on-join session completed and start join session process.
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete, PC));
	SessionInterface->JoinSession(LocalPlayerId.ToSharedRef().Get(), NAME_GameSession, Session);
}

void UMatchmakingEssentialsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
		return;
	}

	// Unbind on-join session completed delegate.
	ensure(SessionInterface.IsValid());
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Failed to join the session."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Log, TEXT("Success to join session. Session Name: %s"), *SessionName.ToString());

		// Bind delegate to listen when the server is ready to travel.
		SessionServerUpdateDelegateHandle = SessionInterface->AddOnSessionServerUpdateDelegate_Handle(FOnSessionServerUpdateDelegate::CreateUObject(this, &ThisClass::OnSessionServerUpdate, PC));
	}
}

void UMatchmakingEssentialsSubsystem::TravelClient(FName SessionName, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		return;
	}

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session Interface is not valid."));
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session is not null."));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Session Info is not valid."));
		return;
	}

	if (!ensure(PC))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. PlayerController is not valid."));
		return;
	}

	// Travel the client to the server via URL.
	FString TravelUrl{};
	if (SessionInterface->GetResolvedConnectString(SessionName, TravelUrl) && !TravelUrl.IsEmpty())
	{
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::MatchFound);
		PC->ClientTravel(TravelUrl, TRAVEL_Absolute);
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the game server. Travel URL is not valid."));
		OnMatchmakingHandle.ExecuteIfBound(EMatchmakingState::FindMatchFailed);
	}
}

void UMatchmakingEssentialsSubsystem::OnSessionServerUpdate(FName SessionName, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		return;
	}

	ensure(SessionInterface.IsValid());
	SessionInterface->ClearOnSessionServerUpdateDelegate_Handle(SessionServerUpdateDelegateHandle);

	// The server is ready. Travel the game clients to the server.
	TravelClient(SessionName, PC);
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

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot leave game session. Session is not valid."));
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot leave game session. LocalPlayer is null."));
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!ensure(LocalPlayerId.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot leave game session. LocalPlayer NetId is not valid."));
		return;
	}

	SessionInterface->LeaveSession(
		LocalPlayerId.ToSharedRef().Get(),
		EAccelByteV2SessionType::GameSession,
		Session->GetSessionIdStr(),
		FOnLeaveSessionComplete::CreateWeakLambda(this, [](bool bWasSuccessful, FString SessionId)
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