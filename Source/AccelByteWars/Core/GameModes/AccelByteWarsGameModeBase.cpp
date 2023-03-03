// Copyright Epic Games, Inc. All Rights Reserved.


#include "AccelByteWarsGameModeBase.h"

#include "AccelByteWarsGameStateBase.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/System/AccelByteWarsGlobals.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameMode);

AAccelByteWarsGameModeBase::AAccelByteWarsGameModeBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AAccelByteWarsGameModeBase::InitGameState()
{
	Super::InitGameState();

	ByteWarsGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ByteWarsGameState = GetGameState<AAccelByteWarsGameStateBase>();

	if (!ensure(ByteWarsGameInstance))
	{
		GAMEMODE_LOG(Warning, TEXT("Game Instance is not (derived from) UAccelByteWarsGameInstance"));
		return;
	}
	if (!ensure(ByteWarsGameState))
	{
		GAMEMODE_LOG(Warning, TEXT("Game State is not (derived from) AAccelByteWarsGameStateBase"));
		return;
	}

	ByteWarsGameInstance->bServerCurrentlyTravelling = false;
}

void AAccelByteWarsGameModeBase::BeginPlay()
{
	// Check if GameSetup have already been set up or not
	if (!ByteWarsGameInstance->GameSetup)
	{
		// have not yet set up, set GameSetup based on launch argument
		FString CodeName;
		FParse::Value(FCommandLine::Get(), TEXT("-GameMode="), CodeName);
		ByteWarsGameInstance->AssignGameMode(CodeName);
	}

	// Setup GameState variables if in GameplayLevel or if DedicatedServer
	if (bIsGameplayLevel || IsRunningDedicatedServer())
	{
		// GameState setup
		ByteWarsGameState->TimeLeft = ByteWarsGameInstance->GameSetup.MatchTime;

		// setup existing players
		for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			PlayerTeamSetup(Iterator->Get());
		}
	}

	// Update game status
	ByteWarsGameState->GameStatus = EGameStatus::AWAITING_PLAYERS;

#pragma region "Server Shutdown Implementation"
#if UE_SERVER || UE_EDITOR
	if (IsRunningDedicatedServer() && bIsGameplayLevel)
	{
		// get specified ShutdownOnFinishedDelay timer value
		FString ShutdownOnFinishedDelayString = "30";
		FParse::Value(FCommandLine::Get(), TEXT("-ShutdownOnFinishedDelay="), ShutdownOnFinishedDelayString);
		ShutdownOnFinishedDelay = FCString::Atoi(*ShutdownOnFinishedDelayString);

		// get specified ShutdownOnOneTeamOrLessDelay timer value
		FString ShutdownOnOneTeamOrLessDelayString = "30";
		FParse::Value(FCommandLine::Get(), TEXT("-ShutdownOnOneTeamOrLessDelay="), ShutdownOnOneTeamOrLessDelayString);
		ShutdownOnOneTeamOrLessDelay = FCString::Atoi(*ShutdownOnOneTeamOrLessDelayString);

		ShutdownDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			GAMEMODE_LOG(Warning, TEXT("Unregistering server from AccelByte Multiplayer Servers (AMS)."));

			AAccelByteWarsGameSession* Session = Cast<AAccelByteWarsGameSession>(GameSession);
			if (!Session)
			{
				GAMEMODE_LOG(Warning, TEXT("Cannot unregister server from AccelByte Multiplayer Servers (AMS), the game session is null."));
				FPlatformMisc::RequestExit(false);
				return;
			}

			// Unregister the server from AccelByte Multiplayer Servers (AMS).
			Session->UnregisterServer();
		});
	}
#endif
#pragma endregion 

	Super::BeginPlay();
}

void AAccelByteWarsGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#pragma region "Lobby Countdown Implementation"
	if (IsRunningDedicatedServer() && !bIsGameplayLevel) 
	{
		// Update countdown
		if (ByteWarsGameState->LobbyStatus == ELobbyStatus::LOBBY_COUNTDOWN_STARTED) 
		{
			if (ByteWarsGameState->LobbyCountdown <= 0)
			{
				ByteWarsGameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
			}
			else
			{
				ByteWarsGameState->LobbyCountdown -= DeltaSeconds;
			}
		}
		// Start server travel
		else if (ByteWarsGameState->LobbyStatus == ELobbyStatus::GAME_STARTED && !ByteWarsGameInstance->bServerCurrentlyTravelling)
		{
			ByteWarsGameInstance->bServerCurrentlyTravelling = true;

			// Delay server travel to let the game clients informed that the game is about to start.
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				GetWorld()->ServerTravel("/Game/ByteWars/Maps/GalaxyWorld/GalaxyWorld");
			}, 1.0f, false);
		}
	}
#pragma endregion

#pragma region "Pre-game countdown implementation"
	if (ByteWarsGameState->GameStatus <= EGameStatus::AWAITING_PLAYERS)
	{
		// check whether all players have logged in or not
		if (GameState->PlayerArray.Num() == ByteWarsGameState->GetRegisteredPlayersNum())
		{
			// start countdown
			ByteWarsGameState->GameStatus = EGameStatus::PRE_GAME_COUNTDOWN_STARTED;
		}
	}
	else if (ByteWarsGameState->GameStatus == EGameStatus::PRE_GAME_COUNTDOWN_STARTED)
	{
		// check if countdown finished
		if (ByteWarsGameState->PreGameCountdown <= 0)
		{
			// finish countdown
			StartGame();
			ByteWarsGameState->GameStatus = EGameStatus::GAME_STARTED;
		}
		// update countdown
		ByteWarsGameState->PreGameCountdown -= DeltaSeconds;
	}
#pragma endregion 

#pragma region "Server Shutdown Implementation"
#if UE_SERVER || UE_EDITOR
	if (IsRunningDedicatedServer() && bIsGameplayLevel)
	{
		// OnFinished
		if (!GetWorldTimerManager().IsTimerActive(OnFinishedTimer))
		{
			if (ByteWarsGameState->GameStatus == EGameStatus::GAME_ENDS)
			{
				GetWorldTimerManager().SetTimer(
					OnFinishedTimer,
					ShutdownDelegate,
					ShutdownOnFinishedDelay,
					false,
					ShutdownOnFinishedDelay);
				GAMEMODE_LOG(Log, TEXT("OnFinishedTimer started. Game will close in %d seconds"), ShutdownOnFinishedDelay);
			}
		}

		// OnOneTeamOrLess
		if (!GetWorldTimerManager().IsTimerActive(OnOneTeamOrLessTimer))
		{
			// check currently connected player TeamId
			int32 TeamId = INDEX_NONE;
			bool bOneTeamConnected = true;

			// check if all player in the same team or if there's no player at all
			for (const TObjectPtr<APlayerState> Player : GameState->PlayerArray)
			{
				if (const AAccelByteWarsPlayerState* ByteWarsPlayerState =
					static_cast<const AAccelByteWarsPlayerState*>(Player))
				{
					if (TeamId == INDEX_NONE)
					{
						TeamId = ByteWarsPlayerState->TeamId;
						continue;
					}
					if (TeamId != ByteWarsPlayerState->TeamId)
					{
						bOneTeamConnected = false;
						break;
					}
				}
			}

			// trigger timer based on above condition
			if (bOneTeamConnected)
			{
				GetWorldTimerManager().SetTimer(
					OnOneTeamOrLessTimer,
					ShutdownDelegate,
					ShutdownOnOneTeamOrLessDelay,
					false,
					ShutdownOnOneTeamOrLessDelay);
				GAMEMODE_LOG(Log, TEXT("OnOnTeamOrLessTimer started. Game will close in %d seconds unless new player logs in"),
					ShutdownOnOneTeamOrLessDelay);
			}
		}
	}
#endif
#pragma endregion 
}

APlayerController* AAccelByteWarsGameModeBase::Login(
	UPlayer* NewPlayer,
	ENetRole InRemoteRole,
	const FString& Portal,
	const FString& Options,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	// setup player if in GameplayLevel and game started
	if ((bIsGameplayLevel || IsRunningDedicatedServer()) && HasMatchStarted())
	{
		PlayerTeamSetup(PlayerController);
	}

#pragma region "Server Shutdown Implementation"
#if UE_SERVER || UE_EDITOR
	if (IsRunningDedicatedServer())
	{
		if (bIsGameplayLevel && GetWorldTimerManager().IsTimerActive(OnOneTeamOrLessTimer))
		{
			GetWorldTimerManager().ClearTimer(OnOneTeamOrLessTimer);
			GAMEMODE_LOG(Log, TEXT("Player logs in, OnOneTeamOrLessTimer cleared"));
		}
		// Start lobby countdown.
		else if (!bIsGameplayLevel && ByteWarsGameState->LobbyStatus == ELobbyStatus::IDLE)
		{
			ByteWarsGameState->LobbyStatus = ELobbyStatus::LOBBY_COUNTDOWN_STARTED;
		}
	}
#endif
#pragma endregion 

	return PlayerController;
}

void AAccelByteWarsGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(NewPlayer->PlayerState))
	{
		if (PlayerState->bShouldKick)
		{
			GameSession->KickPlayer(NewPlayer, FText::FromString("Max player reached"));

			GAMEMODE_LOG(Warning, TEXT("Player did not registered in Teams data. Max registered players reached. Kicking this player"));
		}
	}
}

void AAccelByteWarsGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
#if UE_SERVER || UE_EDITOR
	if (IsRunningDedicatedServer())
	{
		if (bShouldRemovePlayerOnLogoutImmediately && !ByteWarsGameInstance->bServerCurrentlyTravelling)
		{
			const bool bSucceeded = RemovePlayer(Cast<APlayerController>(Exiting));
			GAMEMODE_LOG(Warning, TEXT("Removing player from GameState data. Succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));
		}
	}
#endif
}

int32 AAccelByteWarsGameModeBase::AddPlayerScore(APlayerState* PlayerState, const float InScore, const bool bAddKillCount)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled"));
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		ByteWarsGameState->GetPlayerDataById(PlayerState->GetUniqueId(),GetControllerId(PlayerState));
	if (!PlayerData)
	{
		GAMEMODE_LOG(Warning, TEXT("Player is not in Teams data. Add player to team via AddToTeam. Operation cancelled"));
		return INDEX_NONE;
	}

	// set score in PlayerState and GameState
	const float FinalScore = AccelByteWarsPlayerState->GetScore() + InScore;
	AccelByteWarsPlayerState->SetScore(FinalScore);
	PlayerData->Score = FinalScore;

	// increase kill count
	if (bAddKillCount)
	{
		AccelByteWarsPlayerState->KillCount++;
		PlayerData->KillCount = AccelByteWarsPlayerState->KillCount;
	}

	return AccelByteWarsPlayerState->GetScore();
}

int32 AAccelByteWarsGameModeBase::DecreasePlayerLife(APlayerState* PlayerState, const uint8 Decrement)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled"));
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		ByteWarsGameState->GetPlayerDataById(PlayerState->GetUniqueId(),GetControllerId(PlayerState));
	if (!PlayerData)
	{
		GAMEMODE_LOG(Warning, TEXT("Player is not in Teams data. Add player to team via AddToTeam. Operation cancelled"));
		return INDEX_NONE;
	}

	// decrease life num in PlayerState
	AccelByteWarsPlayerState->NumLivesLeft -= Decrement;

	// match life num in GameState to PlayerState
	PlayerData->NumLivesLeft = AccelByteWarsPlayerState->NumLivesLeft;

	return AccelByteWarsPlayerState->NumLivesLeft;
}

void AAccelByteWarsGameModeBase::ResetGameData()
{
	ByteWarsGameState->Teams.Empty();
}

void AAccelByteWarsGameModeBase::PlayerTeamSetup(APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerController null. Cancelling operation"));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return;
	}

	int32 TeamId = INDEX_NONE;
	const FUniqueNetIdRepl PlayerUniqueId = GetPlayerUniqueNetId(PlayerController);
	const int32 ControllerId = GetControllerId(PlayerState);

	// check for a match in GameState's Teams data
	if (const FGameplayPlayerData* PlayerData =
		ByteWarsGameState->GetPlayerDataById(PlayerState->GetUniqueId(), ControllerId))
	{
		// found, restore data
		TeamId = PlayerData->TeamId;
		PlayerState->SetScore(PlayerData->Score);
		PlayerState->TeamId = TeamId;
		PlayerState->NumLivesLeft = PlayerData->NumLivesLeft;
		PlayerState->KillCount = PlayerData->KillCount;
		PlayerState->TeamColor = ByteWarsGameInstance->GetTeamColor(TeamId);

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
	// flag to kick player if player's data was not found and max players reached (based on registered players)
	else
	{
		if (ByteWarsGameState->GetRegisteredPlayersNum() >= ByteWarsGameInstance->GameSetup.MaxPlayers)
		{
			// kick can happen as early as PostLogin
			PlayerState->bShouldKick = true;
			return;
		}
	}

	// if no match found, assign player to a new team or least populated team
	if (TeamId == INDEX_NONE)
	{
		// If running on DS, get team assignment from Session Info instead.
		if (IsRunningDedicatedServer())
		{
			if (OnGetTeamIdFromSessionDelegate.IsBound()) 
			{
				TeamId = OnGetTeamIdFromSessionDelegate.Execute(PlayerController);
			}
			else 
			{
				GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: Cannot assign team. Delegate to get team id from game session is not bound."));
			}

			// Kick if the player doesn't belong to any team based on Session Info.
			PlayerState->bShouldKick = (TeamId == INDEX_NONE);
		}
		// If running locally, set team assignment manually.
		else 
		{
			switch (ByteWarsGameInstance->GameSetup.GameModeType)
			{
				case EGameModeType::FFA:
					// assign to a new team
					TeamId = ByteWarsGameState->Teams.Num();
					break;
				case EGameModeType::TDM:
					// check if max team reached
					if (ByteWarsGameState->Teams.Num() >= ByteWarsGameInstance->GameSetup.MaxTeamNum)
					{
						// assign to the least populated team
						uint8 CurrentTeamMemberNum = UINT8_MAX;
						TeamId = 0;
						for (const FGameplayTeamData& Team : ByteWarsGameState->Teams)
						{
							if (Team.TeamMembers.Num() < CurrentTeamMemberNum)
							{
								CurrentTeamMemberNum = Team.TeamMembers.Num();
								TeamId = Team.TeamId;
							}
						}
					}
					else
					{
						// assign to a new team
						TeamId = ByteWarsGameState->Teams.Num();
					}
					break;
				default:
					break;
			}
		}

		// reset player's state data
		PlayerState->TeamId = TeamId;
		PlayerState->TeamColor = ByteWarsGameInstance->GetTeamColor(TeamId);
		PlayerState->SetScore(0.0f);
        PlayerState->NumLivesLeft = INDEX_NONE;
        PlayerState->KillCount = 0;

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

	// add player to team
	ByteWarsGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		PlayerState->NumLivesLeft,
		ControllerId,
		PlayerState->GetScore(),
		PlayerState->KillCount);
}

void AAccelByteWarsGameModeBase::AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId)
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerController null. Cancelling operation"));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return;
	}

	const FUniqueNetIdRepl PlayerUniqueId = GetPlayerUniqueNetId(PlayerController);
	const int32 ControllerId = GetControllerId(PlayerState);

	// reset player's state data
	PlayerState->TeamId = TeamId;
	PlayerState->TeamColor = ByteWarsGameInstance->GetTeamColor(TeamId);
	PlayerState->SetScore(0.0f);
	PlayerState->NumLivesLeft = INDEX_NONE;
	PlayerState->KillCount = 0;

	// add player to team
	ByteWarsGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		PlayerState->NumLivesLeft,
		ControllerId,
		PlayerState->GetScore(),
		PlayerState->KillCount);
}

bool AAccelByteWarsGameModeBase::RemovePlayer(const APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerController null. Cancelling operation"));
		return false;
	}

	const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return false;
	}

	const FUniqueNetIdRepl PlayerUniqueId = GetPlayerUniqueNetId(PlayerController);
	const int32 ControllerId = GetControllerId(PlayerState);

	return ByteWarsGameState->RemovePlayerFromTeam(PlayerUniqueId, ControllerId);
}

FUniqueNetIdRepl AAccelByteWarsGameModeBase::GetPlayerUniqueNetId(const APlayerController* PlayerController)
{
	FUniqueNetIdRepl NetId;

	if (PlayerController->IsLocalController())
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			NetId = LocalPlayer->GetPreferredUniqueNetId();
		}
	}
	else
	{
		NetId = PlayerController->PlayerState->GetUniqueId();
	}

	return NetId;
}

int32 AAccelByteWarsGameModeBase::GetControllerId(const APlayerState* PlayerState)
{
	int32 ControllerId = 0;
	if (const APlayerController* PC = PlayerState->GetPlayerController())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			ControllerId = LP->GetLocalPlayerIndex();
		}
	}
	return ControllerId;
}