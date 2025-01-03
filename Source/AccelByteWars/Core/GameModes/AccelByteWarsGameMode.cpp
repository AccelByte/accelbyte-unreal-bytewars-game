// Copyright Epic Games, Inc. All Rights Reserved.


#include "AccelByteWarsGameMode.h"

#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/System/AccelByteWarsGlobals.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameMode);

AAccelByteWarsGameMode::AAccelByteWarsGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	bAllowTickBeforeBeginPlay = false;

	bAllowTickBeforeBeginPlay = false;
}

void AAccelByteWarsGameMode::InitGameState()
{
	Super::InitGameState();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ABGameState = GetGameState<AAccelByteWarsGameState>();

	if (!GameInstance)
	{
		GAMEMODE_LOG(Warning, TEXT("Game Instance is not (derived from) UAccelByteWarsGameInstance"));
		return;
	}
	if (!ABGameState)
	{
		GAMEMODE_LOG(Warning, TEXT("Game State is not (derived from) AAccelByteWarsGameStateBase"));
		return;
	}
}

void AAccelByteWarsGameMode::BeginPlay()
{
	// Check if GameSetup have already been set up or not
	if (!ABGameState->GameSetup)
	{
		FString CodeName;
		FParse::Value(FCommandLine::Get(), TEXT("-GameMode="), CodeName);

		// if launch argument does not exist, will use the first game mode
		ABGameState->AssignGameMode(CodeName);

		/* When transitioning to the listen server (P2P server), the server will initialize 
		 * a new Game Instance and Game State (this is Unreal Engine's default behavior).
		 * Thus, the data set up before transitioning to the listen server will be reset.
		 * Therefore, the delegate below will help to reinitialize the listen server data 
		 * (e.g. assigning game mode through the Game State). */
		if (GetNetMode() == ENetMode::NM_ListenServer && OnInitializeListenServerDelegates.IsBound())
		{
			OnInitializeListenServerDelegates.Broadcast(NAME_GameSession);
		}
	}

	// Setup GameState variables if in GameplayLevel or if DedicatedServer
	if (bIsGameplayLevel || IsServer())
	{
		// setup existing players
		for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			PlayerTeamSetup(Iterator->Get());
		}
	}

	Super::BeginPlay();
}

void AAccelByteWarsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// setup player if in GameplayLevel and game started
	if (bIsGameplayLevel || IsServer())
	{
		PlayerTeamSetup(NewPlayer);
	}
}

// @@@SNIPSTART AccelByteWarsGameMode.cpp-Logout
void AAccelByteWarsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (IsServer())
	{
		if (bShouldRemovePlayerOnLogoutImmediately && !ABGameState->bIsServerTravelling)
		{
			const bool bSucceeded = RemovePlayer(Cast<APlayerController>(Exiting));
			GAMEMODE_LOG(Warning, TEXT("Removing player from GameState data. Succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));
		}
	}

	if (IsRunningDedicatedServer() &&
		ABGameState->PlayerArray.Num() <= 1 &&
		bImmediatelyShutdownWhenEmpty &&
		!ABGameState->bIsServerTravelling)
	{
		CloseGame(TEXT("Last player logs out and bImmediatelyShutdownWhenEmpty was set to true"));
	}
}
// @@@SNIPEND

APawn* AAccelByteWarsGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// do not auto spawn pawn. We have our own handler.
	return nullptr;
}

void AAccelByteWarsGameMode::ResetGameData()
{
	ABGameState->EmptyTeams();
}

void AAccelByteWarsGameMode::PlayerTeamSetup(APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerController null. Canceling operation"));
		return;
	}

	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is invalid. Canceling operation"));
		return;
	}

	AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AbPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is not (derived from) AAccelByteWarsPlayerState. Canceling operation"));
		return;
	}

	int32 TeamId = INDEX_NONE;
	const FUniqueNetIdRepl PlayerUniqueId = AbPlayerState->GetUniqueId();
	const int32 ControllerId = AccelByteWarsUtility::GetControllerId(AbPlayerState);
	
	// check for a match in GameState's Teams data
	if (FGameplayPlayerData* PlayerData =
		ABGameState->GetPlayerDataById(PlayerUniqueId, ControllerId))
	{
		// found
		// stored data doesn't have UniqueNetId while the given data has. Overwrite stored data
		if (AbPlayerState->GetUniqueId().IsValid() && !PlayerData->UniqueNetId.IsValid())
		{
			PlayerData->UniqueNetId = PlayerUniqueId;

			// notify local
			ABGameState->OnNotify_Teams();
		}

		// restore data
		TeamId = PlayerData->TeamId;
		AbPlayerState->SetPlayerName(PlayerData->PlayerName);
		AbPlayerState->AvatarURL = PlayerData->AvatarURL;
		AbPlayerState->SetScore(PlayerData->Score);
		AbPlayerState->TeamId = TeamId;
		AbPlayerState->NumLivesLeft = PlayerData->NumLivesLeft;
		AbPlayerState->KillCount = PlayerData->KillCount;
		AbPlayerState->Deaths = PlayerData->Deaths;
		AbPlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);

#if UE_BUILD_DEVELOPMENT
		const bool bUniqueIdValid = PlayerUniqueId.GetUniqueNetId().IsValid();
		const FString Identity = bUniqueIdValid ?
			PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
		GAMEMODE_LOG(
			Warning,
			TEXT("Found player's (%s [UniqueId: %s]) data in existing PlayerDatas. Assigning team: %d"),
			*Identity,
			*FString(bUniqueIdValid ? "TRUE" : "FALSE"),
			TeamId);
#endif
	}
	else
	{
		if (OnPlayerPostLoginDelegates.IsBound() && IsServer())
		{
			// notify client
			AbPlayerState->bPendingTeamAssignment = true;
			// notify self if running as listen server
			if (!IsRunningDedicatedServer())
			{
				AbPlayerState->RepNotify_PendingTeamAssignment();
			}

			// tutorial module active, let the subsystem handle the team assignment logic
			OnPlayerPostLoginDelegates.Broadcast(PlayerController);

			return;
		}
		else
		{
			// kick player if max player reached
			if (ABGameState->GetRegisteredPlayersNum() >= ABGameState->GameSetup.MaxPlayers)
			{
				// kick can happen as early as PostLogin
				GAMEMODE_LOG(Warning, TEXT("Player did not registered in Teams data. Max registered players reached (%d / %d). Kicking this player"),
					ABGameState->GetRegisteredPlayersNum(),
					ABGameState->GameSetup.MaxPlayers);
				GameSession->KickPlayer(PlayerController, FText::FromString("Max player reached"));
				return;
			}
			// assign team manually
			else
			{
				AssignTeamManually(TeamId);

				// reset player's state data
				AbPlayerState->SetPlayerName(TEXT(""));
				AbPlayerState->AvatarURL = TEXT("");
				AbPlayerState->TeamId = TeamId;
				AbPlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);
				AbPlayerState->SetScore(0.0f);
				AbPlayerState->NumLivesLeft = INDEX_NONE;
				AbPlayerState->KillCount = 0;
				AbPlayerState->Deaths = 0;

#if UE_BUILD_DEVELOPMENT
				const bool bUniqueIdValid = PlayerUniqueId.GetUniqueNetId().IsValid();
				const FString Identity = bUniqueIdValid ?
					PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
				GAMEMODE_LOG(
					Warning,
					TEXT("No player's (%s [UniqueId: %s]) data found. Assigning team: %d"),
					*Identity,
					*FString(bUniqueIdValid ? "TRUE" : "FALSE"),
					TeamId);
#endif
			}
		}
	}

	ABGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		AbPlayerState->NumLivesLeft,
		ControllerId,
		AbPlayerState->GetScore(),
		AbPlayerState->KillCount,
		AbPlayerState->Deaths);
}

void AAccelByteWarsGameMode::DelayedPlayerTeamSetupWithPredefinedData(APlayerController* PlayerController)
{
	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is invalid. Canceling operation"));
		return;
	}

	AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AbPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is not (derived from) AAccelByteWarsPlayerState. Canceling operation"));
		return;
	}

	AbPlayerState->bPendingTeamAssignment = false;

	// notify self if running as listen server
	if (!IsRunningDedicatedServer())
	{
		AbPlayerState->RepNotify_PendingTeamAssignment();
	}

	const FUniqueNetIdRepl PlayerUniqueId = AbPlayerState->GetUniqueId();
	const int32 ControllerId = AccelByteWarsUtility::GetControllerId(AbPlayerState);

	// reset player's state data
	AbPlayerState->TeamColor = GameInstance->GetTeamColor(AbPlayerState->TeamId);
	AbPlayerState->SetScore(0.0f);
	AbPlayerState->NumLivesLeft = INDEX_NONE;
	AbPlayerState->KillCount = 0;
	AbPlayerState->Deaths = 0;

	ABGameState->AddPlayerToTeam(
		AbPlayerState->TeamId,
		PlayerUniqueId,
		AbPlayerState->NumLivesLeft,
		ControllerId,
		AbPlayerState->GetScore(),
		AbPlayerState->KillCount,
		AbPlayerState->Deaths,
		AbPlayerState->GetPlayerName(),
		AbPlayerState->AvatarURL);
}

void AAccelByteWarsGameMode::AssignTeamManually(int32& InOutTeamId) const
{
	switch (ABGameState->GameSetup.GameModeType)
	{
	case EGameModeType::FFA:
		InOutTeamId = INDEX_NONE;

		// Assign to empty team
		for (const int EmptyTeamId : ABGameState->GetEmptyTeamIds())
		{
			InOutTeamId = EmptyTeamId;
			break;
		}
		break;
	case EGameModeType::TDM:
		InOutTeamId = INDEX_NONE;

		// Assign to empty team
		for (const int EmptyTeamId : ABGameState->GetEmptyTeamIds())
		{
			InOutTeamId = EmptyTeamId;
			break;
		}

		if (InOutTeamId == INDEX_NONE)
		{
			// Assign to least populated team.
			if (ABGameState->Teams.Num() >= ABGameState->GameSetup.MaxTeamNum)
			{
				// The least populated team should less than the maximum players per team.
				uint8 CurrentTeamMemberNum = ABGameState->GameSetup.MaxPlayers / ABGameState->GameSetup.MaxTeamNum;
				InOutTeamId = INDEX_NONE;
				for (const FGameplayTeamData& Team : ABGameState->Teams)
				{
					if (Team.TeamMembers.Num() < CurrentTeamMemberNum)
					{
						CurrentTeamMemberNum = Team.TeamMembers.Num();
						InOutTeamId = Team.TeamId;
					}
				}
			}
			// Assign to a new team
			else
			{
				InOutTeamId = ABGameState->Teams.Num();
			}
		}
		break;
	default:
		break;
	}
}

void AAccelByteWarsGameMode::ServerTravel(TSoftObjectPtr<UWorld> Level)
{
	const FString Url = Level.GetLongPackageName();
	DelayedServerTravel(Url);
}

void AAccelByteWarsGameMode::DelayedServerTravel(const FString& URL) const
{
	if (ABGameState->bIsServerTravelling)
	{
		// if true, server already traveling, cancel operation
		return;
	}

	ABGameState->bIsServerTravelling = true;
	ABGameState->OnNotify_IsServerTravelling();

	// remove empty team
	ABGameState->RemoveEmptyTeam();

	// Delay server travel to let the game clients informed that the game is about to start.
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, URL]()
	{
		GetWorld()->ServerTravel(URL);
	}, 1.0f, false);
}

// @@@SNIPSTART AccelByteWarsGameMode.cpp-CloseGame
void AAccelByteWarsGameMode::CloseGame(const FString& Reason) const
{
	GAMEMODE_LOG(Warning, TEXT("Unregistering or shutting down server with reason: %s."), *Reason);

	if (!IsRunningDedicatedServer())
	{
		GAMEMODE_LOG(Warning, TEXT("Not a Dedicated Server, shutdown canceled"));
		return;
	}

	if (OnPreGameShutdown.IsBound())
	{
		OnPreGameShutdown.Broadcast(TDelegate<void()>::CreateUObject(this, &ThisClass::CloseGameInternal));
	}
	else
	{
		CloseGameInternal();
	}
}
// @@@SNIPEND

void AAccelByteWarsGameMode::SetImmediatelyShutdownWhenEmpty(const bool bAllow) const
{
	bImmediatelyShutdownWhenEmpty = bAllow;

	GAMEMODE_LOG(Log, TEXT("bImmediatelyShutdownWhenEmpty set to %s"), *FString(bAllow ? "TRUE" : "FALSE"));
}

void AAccelByteWarsGameMode::AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId)
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerController null. Canceling operation"));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerState is not (derived from) AAccelByteWarsPlayerState. Canceling operation"));
		return;
	}

	const FUniqueNetIdRepl PlayerUniqueId = PlayerState->GetUniqueId();
	const int32 ControllerId = AccelByteWarsUtility::GetControllerId(PlayerState);

	// reset player's state data
	PlayerState->TeamId = TeamId;
	PlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);
	PlayerState->SetScore(0.0f);
	PlayerState->NumLivesLeft = INDEX_NONE;
	PlayerState->KillCount = 0;
	PlayerState->Deaths = 0;

	// add player to team
	ABGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		PlayerState->NumLivesLeft,
		ControllerId,
		PlayerState->GetScore(),
		PlayerState->KillCount,
		PlayerState->Deaths);
}

bool AAccelByteWarsGameMode::RemovePlayer(const APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerController null. Canceling operation"));
		return false;
	}

	const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerState is not (derived from) AAccelByteWarsPlayerState. Canceling operation"));
		return false;
	}

	const FUniqueNetIdRepl PlayerUniqueId = PlayerState->GetUniqueId();
	const int32 ControllerId = AccelByteWarsUtility::GetControllerId(PlayerState);

	return ABGameState->RemovePlayerFromTeam(PlayerUniqueId, ControllerId);
}

bool AAccelByteWarsGameMode::IsServer() const
{
	/**
	 * Using THIS GetNetMode will return as client even on a listen server mode.
	 * Uses the World's NetMode as a workaround
	 */
	const ENetMode NetMode = GetWorld()->GetNetMode();
	return NetMode == ENetMode::NM_DedicatedServer || NetMode == ENetMode::NM_ListenServer;
}

void AAccelByteWarsGameMode::SetupSimulateServerCrashCountdownValue(const FString& SimulateServerCrashArg)
{
	if (!IsRunningDedicatedServer()) 
	{
		return;
	}

	bShouldSimulateServerCrash = false;

	// Check simulate server crash argument from launch param.
	const FString CmdArgs = FCommandLine::Get();
	if (!CmdArgs.Contains(SimulateServerCrashArg, ESearchCase::IgnoreCase))
	{
		return;
	}

	// Set the default simulate server crash countdown.
	ABGameState->SimulateServerCrashCountdown = 20;

	// Override simulate server crash countdown value with the launch param value if any.
	FString ParamValueStr;
	int32 ParamValue;
	FParse::Value(*CmdArgs, *FString::Printf(TEXT("%s="), *SimulateServerCrashArg), ParamValueStr);
	if (!ParamValueStr.IsEmpty() && ParamValueStr.IsNumeric())
	{
		ParamValue = FCString::Atoi(*ParamValueStr);
		if (ParamValue >= 0)
		{
			ABGameState->SimulateServerCrashCountdown = ParamValue;
		}
	}

	bShouldSimulateServerCrash = true;
}

void AAccelByteWarsGameMode::SimulateServerCrashCountdownCounting(const float& DeltaSeconds) const
{
	if (!IsRunningDedicatedServer() || !bShouldSimulateServerCrash)
	{
		return;
	}

	float Countdown = ABGameState->SimulateServerCrashCountdown;
	if (Countdown <= 0)
	{
		GAMEMODE_LOG(Warning, TEXT("Server is simulated to crash."));

		// Simulating crash by accessing a null pointer.
		const APlayerController* CrashPC = nullptr;
		CrashPC->GetLocalPlayer()->GetPreferredUniqueNetId();

		return;
	}

	GAMEMODE_LOG(Warning, TEXT("Simulating server crash in: %ds"), (int32)Countdown);
	ABGameState->SimulateServerCrashCountdown -= DeltaSeconds;
}

// @@@SNIPSTART AccelByteWarsGameMode.cpp-CloseGameInternal
void AAccelByteWarsGameMode::CloseGameInternal() const
{
	AAccelByteWarsGameSession* Session = Cast<AAccelByteWarsGameSession>(GameSession);
	if (!Session)
	{
		GAMEMODE_LOG(Warning, TEXT("The game session is null. Shutting down immediately."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	// Unregister the server.
	Session->UnregisterServer();
}
// @@@SNIPEND

