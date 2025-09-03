// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/GameModes/AccelByteWarsInGameGameMode.h"

#include <algorithm>

#include "Core/Actor/AccelByteWarsFxActor.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/Player/AccelByteWarsBotController.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/Utilities/AccelByteWars2DProbabilityDistribution.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/Actor/AccelByteWarsSpawner.h"
#include "Core/Settings/SpawnerConfigurationDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Core/Player/AccelByteWarsAttributeSet.h"

#define NULLPTR_CHECK(Object) \
	if (!Object)              \
		return;

AAccelByteWarsInGameGameMode::AAccelByteWarsInGameGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	bAllowTickBeforeBeginPlay = false;
}

void AAccelByteWarsInGameGameMode::InitGameState()
{
	Super::InitGameState();

	ABInGameGameState = GetGameState<AAccelByteWarsInGameGameState>();

	if (!ensure(ABInGameGameState))
	{
		GAMEMODE_LOG(Warning, TEXT("Game State is not (derived from) AAccelByteWarsGalaxyWorldGameStateBase"));
		return;
	}
}

void AAccelByteWarsInGameGameMode::BeginPlay()
{
	// Setup game data
	ABInGameGameState->GameStatus = EGameStatus::AWAITING_PLAYERS;
	ABInGameGameState->TimeLeft = ABInGameGameState->GameSetup.MatchTime;

#pragma region "Server Shutdown Implementation"
#if UE_SERVER || UE_EDITOR
	if (IsRunningDedicatedServer() && bIsGameplayLevel)
	{
		SetupShutdownCountdownsValue();

		/* Set simulate server crash countdown.
		 * The countdown will only be started in the gameplay level.*/
		SetupSimulateServerCrashCountdownValue(FString("-SIM_SERVER_CRASH_GAMEPLAY"));
	}
#endif
#pragma endregion

	Super::BeginPlay();
}

void AAccelByteWarsInGameGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up the Asteroid spawner
	if (Spawner)
	{
		Spawner->StopSpawning();
		Spawner = nullptr;
	}

	Super::EndPlay(EndPlayReason);

	if (!ABInGameGameState)
	{
		// By Unreal's GameMode design, this shouldn't happen. Just in case.
		GAMEMODE_LOG(Warning, TEXT("Game State is null right when map about to close."));
		return;
	}

	if (ABInGameGameState->GameStatus == EGameStatus::INVALID)
	{
		// Game ends properly.
		return;
	}

	// Game ends unexpectedly. Notify game ends here.
	if (OnGameEndsDelegate.IsBound())
	{
		OnGameEndsDelegate.Broadcast(GAME_END_REASON_TERMINATED);
	}
	GAMEMODE_LOG(
		Log,
		TEXT("Game ending unexpectedly. Current status: %s, EndPlayReason: %s"),
		*UEnum::GetValueAsString(ABInGameGameState->GameStatus),
		*UEnum::GetValueAsString(EndPlayReason));
}

// @@@SNIPSTART AccelByteWarsInGameGameMode.cpp-Tick
// @@@MULTISNIP AwaitingPlayerState {"selectedLines": ["1-2", "8-28", "93"]}
// @@@MULTISNIP AwaitingPlayerMidGameState {"selectedLines": ["1-2", "37-52", "93"]}
// @@@MULTISNIP GameStartedState {"selectedLines": ["1-2", "53-71", "93"]}
// @@@MULTISNIP GameEndsState {"selectedLines": ["1-2", "79-89", "93"]}
void AAccelByteWarsInGameGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (ABInGameGameState->GameStatus)
	{
		case EGameStatus::IDLE:
		case EGameStatus::AWAITING_PLAYERS:
			// Check if all registered players have re-entered the server
			if (ABInGameGameState->PlayerArray.Num() == ABInGameGameState->GetRegisteredPlayersNum())
			{
				ABInGameGameState->GameStatus = EGameStatus::PRE_GAME_COUNTDOWN_STARTED;
				FillEmptySlotWithBot();
				if (IsRunningDedicatedServer())
				{
					// reset NotEnoughPlayerCountdown
					SetupShutdownCountdownsValue();
				}
			}
			else
			{
				// Use NotEnoughPlayerCountdown as a countdown to wait for all registered players to reconnect to the DS.
				if (IsRunningDedicatedServer())
				{
					NotEnoughPlayerCountdownCounting(DeltaSeconds);
				}
			}
			break;
		case EGameStatus::PRE_GAME_COUNTDOWN_STARTED:
			ABInGameGameState->PreGameCountdown -= DeltaSeconds;
			if (ABInGameGameState->PreGameCountdown <= 0)
			{
				ABInGameGameState->GameStatus = EGameStatus::GAME_STARTED;
				StartGame();
			}
			break;
		case EGameStatus::AWAITING_PLAYERS_MID_GAME:
			if (IsRunningDedicatedServer())
			{
				SimulateServerCrashCountdownCounting(DeltaSeconds);

				if (ShouldStartNotEnoughPlayerCountdown())
				{
					NotEnoughPlayerCountdownCounting(DeltaSeconds);
				}
				else
				{
					ABInGameGameState->GameStatus = EGameStatus::GAME_STARTED;
					SetupShutdownCountdownsValue();
				}
			}
			break;
		case EGameStatus::GAME_STARTED:
			if (IsRunningDedicatedServer())
			{
				SimulateServerCrashCountdownCounting(DeltaSeconds);

				if (ShouldStartNotEnoughPlayerCountdown())
				{
					ABInGameGameState->GameStatus = EGameStatus::AWAITING_PLAYERS_MID_GAME;
				}
			}

			// Gameplay timer
			ABInGameGameState->TimeLeft -= DeltaSeconds;
			if (ABInGameGameState->TimeLeft <= 0)
			{
				ABInGameGameState->TimeLeft = 0;
				EndGame(GAME_END_REASON_TIMES_UP);
			}
			break;
		case EGameStatus::GAME_ENDS_DELAY:
			GameEndsDelay -= DeltaSeconds;
			if (GameEndsDelay <= 0)
			{
				ABInGameGameState->GameStatus = EGameStatus::GAME_ENDS;
			}
			break;
		case EGameStatus::GAME_ENDS:
			if (IsRunningDedicatedServer() && ABInGameGameState->GameSetup.GameEndsShutdownCountdown != INDEX_NONE)
			{
				ABInGameGameState->PostGameCountdown -= DeltaSeconds;
				if (ABInGameGameState->PostGameCountdown <= 0)
				{
					ABInGameGameState->GameStatus = EGameStatus::INVALID;
					CloseGame("Game finished");
				}
			}
			break;
		case EGameStatus::INVALID:
			break;
		default:;
	}
}
// @@@SNIPEND

void AAccelByteWarsInGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	APlayerState* PlayerState = NewPlayer->PlayerState;
	if (!PlayerState)
	{
		return;
	}

	AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AbPlayerState)
	{
		return;
	}

	// Setup player if game has started and is not pending
	if (ABInGameGameState->HasGameStarted() && HasMatchStarted() && !AbPlayerState->bPendingTeamAssignment)
	{
		SpawnAndPossesPawn(PlayerState);
	}

	// retrieve player's equipped items
	AbPlayerState->ClientRetrieveEquippedItems();

	// Notify other if the match has already started
	switch (ABInGameGameState->GameStatus)
	{
		case EGameStatus::GAME_STARTED:
		case EGameStatus::AWAITING_PLAYERS_MID_GAME:
			if (OnPlayerEnteredMatch.IsBound())
			{
				OnPlayerEnteredMatch.Broadcast(AbPlayerState->GetUniqueId().GetUniqueNetId());
			}
			break;
	}
}

void AAccelByteWarsInGameGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!Exiting)
	{
		return;
	}

	// Check if this was a human player (not a bot)
	if (IsValid(Exiting->PlayerState) && Cast<AAccelByteWarsPlayerController>(Exiting))
	{
		const AAccelByteWarsPlayerState* ExitingPlayerState = Cast<AAccelByteWarsPlayerState>(Exiting->PlayerState);
		if (ExitingPlayerState && ExitingPlayerState->GetUniqueId().IsValid())
		{
			GAMEMODE_LOG(Log, TEXT("Human player disconnected: %s"), *ExitingPlayerState->GetPlayerName());
		}

		// Check if we still have valid players after this logout
		if (!HasValidPlayers())
		{
			GAMEMODE_LOG(Warning, TEXT("No valid players remaining after logout. Game may need to end."));

			// Only trigger automatic shutdown if configured and we're in a running state
			if (ShouldStartNotEnoughPlayerCountdown() && (ABInGameGameState->GameStatus == EGameStatus::GAME_STARTED || ABInGameGameState->GameStatus == EGameStatus::AWAITING_PLAYERS_MID_GAME))
			{
				ABInGameGameState->GameStatus = EGameStatus::AWAITING_PLAYERS_MID_GAME;
				GAMEMODE_LOG(Log, TEXT("Transitioning to AWAITING_PLAYERS_MID_GAME due to insufficient players"));
			}
		}
	}
}

void AAccelByteWarsInGameGameMode::DelayedPlayerTeamSetupWithPredefinedData(APlayerController* PlayerController)
{
	Super::DelayedPlayerTeamSetupWithPredefinedData(PlayerController);

	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		return;
	}

	const AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AbPlayerState)
	{
		return;
	}

	if (
		ABInGameGameState->HasGameStarted() && HasMatchStarted() && !AbPlayerState->bPendingTeamAssignment && !ABInGameGameState->HasGameEnded())
	{
		SpawnAndPossesPawn(PlayerState);
	}
}

int32 AAccelByteWarsInGameGameMode::AddPlayerScore(
	APlayerState* PlayerState,
	float InScore,
	bool bAddKillCount)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled"));
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		ABInGameGameState->GetPlayerDataById(PlayerState->GetUniqueId(), AccelByteWarsUtility::GetControllerId(PlayerState));
	if (!PlayerData)
	{
		GAMEMODE_LOG(Warning, TEXT("Player is not in Teams data. Add player to team via AddToTeam. Operation cancelled"));
		return INDEX_NONE;
	}

	// set score in PlayerState and GameState
	const float FinalScore = AccelByteWarsPlayerState->GetScore() + (InScore * AccelByteWarsPlayerState->GetAttributeSet()->ScoreMultiplier.GetCurrentValue());
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

int32 AAccelByteWarsInGameGameMode::DecreasePlayerLife(APlayerState* PlayerState, uint8 Decrement)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled"));
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		ABInGameGameState->GetPlayerDataById(PlayerState->GetUniqueId(), AccelByteWarsUtility::GetControllerId(PlayerState));
	if (!PlayerData)
	{
		GAMEMODE_LOG(Warning, TEXT("Player is not in Teams data. Add player to team via AddToTeam. Operation cancelled"));
		return INDEX_NONE;
	}

	// decrease life num in PlayerState
	AccelByteWarsPlayerState->NumLivesLeft -= Decrement;
	AccelByteWarsPlayerState->NumKilledAttemptInSingleLifetime = 0;
	AccelByteWarsPlayerState->Deaths += Decrement;

	// match life num in GameState to PlayerState
	PlayerData->NumLivesLeft = AccelByteWarsPlayerState->NumLivesLeft;
	PlayerData->NumKilledAttemptInSingleLifetime = AccelByteWarsPlayerState->NumKilledAttemptInSingleLifetime;
	PlayerData->Deaths = AccelByteWarsPlayerState->Deaths;

	return AccelByteWarsPlayerState->NumLivesLeft;
}

int32 AAccelByteWarsInGameGameMode::IncrementPlayerLife(APlayerState* PlayerState, uint8 Increment)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled"));
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		ABInGameGameState->GetPlayerDataById(PlayerState->GetUniqueId(), AccelByteWarsUtility::GetControllerId(PlayerState));
	if (!PlayerData)
	{
		GAMEMODE_LOG(Warning, TEXT("Player is not in Teams data. Add player to team via AddToTeam. Operation cancelled"));
		return INDEX_NONE;
	}

	AccelByteWarsPlayerState->NumLivesLeft += Increment;
	PlayerData->NumLivesLeft = AccelByteWarsPlayerState->NumLivesLeft;

	ABInGameGameState->OnNotify_Teams();

	return AccelByteWarsPlayerState->NumLivesLeft;
}

void AAccelByteWarsInGameGameMode::EndGame(const FString Reason)
{
	ABInGameGameState->GameStatus = EGameStatus::GAME_ENDS_DELAY;

	if (OnGameEndsDelegate.IsBound())
	{
		OnGameEndsDelegate.Broadcast(Reason);
	}

	GAMEMODE_LOG(Log, TEXT("Game ends with reason: %s."), *Reason);
}

bool AAccelByteWarsInGameGameMode::HasValidPlayers() const
{
	if (!ABInGameGameState)
	{
		GAMEMODE_LOG(Warning, TEXT("GameState is null when checking for valid players"));
		return false;
	}

	const int32 HumanPlayerCount = GetConnectedHumanPlayerCount();

	const bool bHasHumanPlayers = HumanPlayerCount > 0;
	const bool bHasRegisteredPlayers = ABInGameGameState->GetRegisteredPlayersNum() > 0;
	const bool bIsAwaitingReconnection = (ABInGameGameState->GameStatus == EGameStatus::AWAITING_PLAYERS || ABInGameGameState->GameStatus == EGameStatus::AWAITING_PLAYERS_MID_GAME);

	return bHasHumanPlayers || (bHasRegisteredPlayers && bIsAwaitingReconnection);
}

int32 AAccelByteWarsInGameGameMode::GetConnectedHumanPlayerCount() const
{
	if (!ABInGameGameState)
	{
		return 0;
	}

	int32 HumanPlayerCount = 0;

	for (const TObjectPtr<APlayerState>& PlayerState : ABInGameGameState->PlayerArray)
	{
		if (!PlayerState)
		{
			continue;
		}

		if (const AController* PlayerController = PlayerState->GetOwningController())
		{
			if (Cast<AAccelByteWarsPlayerController>(PlayerController))
			{
				if (PlayerState->GetUniqueId().IsValid())
				{
					HumanPlayerCount++;
				}
			}
		}
	}

	return HumanPlayerCount;
}

void AAccelByteWarsInGameGameMode::OnShipDestroyed(
	UAccelByteWarsGameplayObjectComponent* Ship,
	const float MissileScore,
	AController* SourcePlayerController)
{
	// NOTE: Non missile hitting player will don't have SourcePlayerController, you need to always check whether
	// the instigator is from player or not.
	AAccelByteWarsPlayerState* SourcePlayerState = nullptr;
	if (SourcePlayerController != nullptr)
	{
		SourcePlayerState = Cast<AAccelByteWarsPlayerState>(SourcePlayerController->PlayerState);
		NULLPTR_CHECK(SourcePlayerState);
	}

	const AActor* ShipActor = Ship->GetOwner();
	NULLPTR_CHECK(ShipActor);

	const APawn* ShipOwner = Cast<APawn>(ShipActor->GetNetOwner());
	NULLPTR_CHECK(ShipOwner);

	const AController* ShipPC = Cast<AController>(ShipOwner->Controller);
	NULLPTR_CHECK(ShipPC);

	AAccelByteWarsPlayerState* ShipPlayerState = Cast<AAccelByteWarsPlayerState>(ShipPC->PlayerState);
	NULLPTR_CHECK(ShipPlayerState);

	// Broadcast on-player die event.
	ABInGameGameState->MulticastOnPlayerDie(ShipPlayerState, ShipActor->GetActorLocation(), SourcePlayerState);

	// Let the ABPawn know they've been destroyed to cleanup UI
	const AAccelByteWarsPlayerPawn* ABPawn = Cast<AAccelByteWarsPlayerPawn>(ShipOwner);
	NULLPTR_CHECK(ABPawn);
	const_cast<AAccelByteWarsPlayerPawn*>(ABPawn)->Client_OnDestroyed();

	// Reset Missile Fired Count
	ShipPlayerState->MissilesFired = 0;

	// FX logic
	OnShipDestroyedFX(SourcePlayerController, ShipOwner->GetTransform(), ShipPlayerState);

	// Adjust player's var
	DecreasePlayerLife(ShipPlayerState, 1);
	// If friendly, do nothing
	if (SourcePlayerState && SourcePlayerState->TeamId != ShipPlayerState->TeamId)
	{
		AddPlayerScore(SourcePlayerState, MissileScore);
	}

	// Trigger OnNotify manually if this is a P2P host
	if (!IsRunningDedicatedServer())
	{
		ABInGameGameState->OnNotify_Teams();
	}

	/**
	 * Respawn ship if there's still lives left
	 * Respawn first before destroying previous pawn.
	 * This way, the new pawn will be guaranteed spawn location that is not near the previous one
	 */
	SpawnAndPossesPawn(ShipPlayerState);

	// Destroy ship and all of its attached actor
	TArray<AActor*> Actors;
	Ship->GetOwner()->GetAttachedActors(Actors);
	for (AActor* Actor : Actors)
	{
		Actor->Destroy();
	}
	Ship->GetOwner()->Destroy();

	// If only 1 team left, end game
	if (GetLivingTeamCount() <= 1)
	{
		EndGame(GAME_END_REASON_LAST_TEAM);
		return;
	}

	// If score limit reached, end game
	if (SourcePlayerState)
	{
		FGameplayTeamData TeamData{};
		float TeamScore = 0;
		int32 TeamLivesLeft = 0, TeamKillCount = 0, TeamDeaths = 0;
		ABInGameGameState->GetTeamDataByTeamId(SourcePlayerState->TeamId, TeamData, TeamScore, TeamLivesLeft, TeamKillCount, TeamDeaths);
		if (ABInGameGameState->GameSetup.ScoreLimit >= 0)
		{
			if (TeamScore >= ABInGameGameState->GameSetup.ScoreLimit)
			{
				EndGame("Score limit reached");
				return;
			}
		}
	}
}

void AAccelByteWarsInGameGameMode::OnMissileDestroyed_Implementation(FVector Location,
	UAccelByteWarsGameplayObjectComponent* HitObject, FLinearColor MissileColour, AActor* MissileOwner)
{
	FActorSpawnParameters Parameters;
	Parameters.Owner = MissileOwner;
	AAccelByteWarsFxActor* FxActor = GetWorld()->SpawnActor<AAccelByteWarsFxActor>(DestroyedFxActor, Location, FRotator::ZeroRotator, Parameters);
	if (FxActor)
	{
		FxActor->SetNiagaraFxColor(MissileColour);
	}
}

void AAccelByteWarsInGameGameMode::IncreasePlayerKilledAttempt(const AController* TargetPlayer)
{
	NULLPTR_CHECK(TargetPlayer);

	AAccelByteWarsPlayerState* TargetPlayerState = Cast<AAccelByteWarsPlayerState>(TargetPlayer->PlayerState);
	NULLPTR_CHECK(TargetPlayerState);

	FGameplayPlayerData* PlayerData = ABInGameGameState->GetPlayerDataById(TargetPlayerState->GetUniqueId(), AccelByteWarsUtility::GetControllerId(TargetPlayerState));
	NULLPTR_CHECK(PlayerData);

	TargetPlayerState->NumKilledAttemptInSingleLifetime++;
	PlayerData->NumKilledAttemptInSingleLifetime = TargetPlayerState->NumKilledAttemptInSingleLifetime;
}

void AAccelByteWarsInGameGameMode::InstaKillPlayers(const TArray<APlayerState*>& PlayerStates)
{
	// Treat as suicide
	for (APlayerState* PlayerState : PlayerStates)
	{
		if (!PlayerState)
		{
			continue;
		}

		APlayerController* PlayerController = PlayerState->GetPlayerController();
		if (!PlayerController)
		{
			return;
		}

		UPlayer* Player = PlayerState->GetNetOwningPlayer();
		if (!Player)
		{
			continue;
		}

		// Find ship from active game objects
		for (UAccelByteWarsGameplayObjectComponent* Component : ABInGameGameState->ActiveGameObjects)
		{
			if (Component && Component->ObjectType == EGameplayObjectType::SHIP && Component->GetOwner()->GetNetOwningPlayer() == Player)
			{
				OnShipDestroyed(Component, 0, PlayerController);
				break;
			}
		}
	}
}

void AAccelByteWarsInGameGameMode::RemoveFromActiveGameObjects(AActor* DestroyedActor)
{
	if (UAccelByteWarsGameplayObjectComponent* Component =
			DestroyedActor->FindComponentByClass<UAccelByteWarsGameplayObjectComponent>())
	{
		ABInGameGameState->ActiveGameObjects.Remove(Component);
	}
}

void AAccelByteWarsInGameGameMode::OnShipDestroyedFX_Implementation(AController* SourcePlayerController,
	const FTransform ShipTransform, AAccelByteWarsPlayerState* ShipPlayerState)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = ShipPlayerState->GetOwningController();
	AAccelByteWarsFxActor* DestroyFx = GetWorld()->SpawnActor<AAccelByteWarsFxActor>(ShipDestroyFxClass, ShipTransform, SpawnParameters);
	if (DestroyFx)
	{
		DestroyFx->SetNiagaraFxColor(ShipPlayerState->TeamColor);
	}
	if (AAccelByteWarsPlayerPawn* DestroyedPawn = Cast<AAccelByteWarsPlayerPawn>(ShipPlayerState->GetOwner()))
	{
		DestroyedPawn->PulseCameraBackground();
	}
}

#pragma region "Gameplay logic"
void AAccelByteWarsInGameGameMode::StartGame()
{
	// Spawn planets
	SpawnPlanets();

	// Spawn player start
	for (const FVector& PlayerStart : PlayerStartPoints)
	{
		GetWorld()->SpawnActor<APlayerStart>(PlayerStart, FRotator::ZeroRotator);
	}

	// Spawn and posses ships
	for (APlayerState* PlayerState : ABInGameGameState->PlayerArray)
	{
		SpawnAndPossesPawn(PlayerState);
	}
	AAccelByteWarsInGameGameState* InGameGameState = GetGameState<AAccelByteWarsInGameGameState>();
	// Create AccelByteWarsSpawner using DataAsset configuration
	if (!Spawner && HasAuthority() && InGameGameState && InGameGameState->GameSetup.SpawnerConfiguration)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;

		Spawner = GetWorld()->SpawnActor<AAccelByteWarsSpawner>(
			AAccelByteWarsSpawner::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);

		USpawnerConfigurationDataAsset* SpawnerConfiguration = InGameGameState->GameSetup.SpawnerConfiguration;

		if (Spawner)
		{
			if (SpawnerConfiguration)
			{
				// Apply configuration from DataAsset
				SpawnerConfiguration->ApplyConfigurationToSpawner(Spawner);
				UE_LOG(LogTemp, Log, TEXT("AccelByteWarsInGameGameMode: Created Asteroid spawner and applied DataAsset configuration"));
				if (InGameGameState->HasGameStarted() && SpawnerConfiguration->Configuration.bAutoStartSpawning)
				{
					Spawner->StartSpawning();
				}
				else
				{
					Spawner->StopSpawning();
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AccelByteWarsInGameGameMode: Failed to create Asteroid spawner"));
		}
	}

	// Notify that the game has started
	if (OnGameStartedDelegates.IsBound())
	{
		OnGameStartedDelegates.Broadcast();
	}
	for (const APlayerState* Player : GameState->PlayerArray)
	{
		if (!Player)
		{
			continue;
		}

		if (OnPlayerEnteredMatch.IsBound())
		{
			OnPlayerEnteredMatch.Broadcast(Player->GetUniqueId().GetUniqueNetId());
		}
	}
}

void AAccelByteWarsInGameGameMode::SetupGameplayObject(AActor* Object) const
{
	Object->SetReplicates(true);
	if (UAccelByteWarsGameplayObjectComponent* Component =
			Cast<UAccelByteWarsGameplayObjectComponent>(Object->GetComponentByClass(UAccelByteWarsGameplayObjectComponent::StaticClass())))
	{
		ABInGameGameState->ActiveGameObjects.Add(Component);
	}

	Object->OnDestroyed.AddDynamic(this, &ThisClass::RemoveFromActiveGameObjects);
}

int32 AAccelByteWarsInGameGameMode::GetLivingTeamCount() const
{
	// check currently connected player TeamId
	TArray<int32> ActiveTeams;

	// check if all player in the same team or if there's no player at all
	for (const TObjectPtr<APlayerState>& Player : ABInGameGameState->PlayerArray)
	{
		if (const AAccelByteWarsPlayerState* ByteWarsPlayerState =
				static_cast<const AAccelByteWarsPlayerState*>(Player))
		{
			// if player's have no lives left, skip
			if (ByteWarsPlayerState->NumLivesLeft <= 0)
			{
				continue;
			}

			// set TeamId to the first 'living' player
			ActiveTeams.AddUnique(ByteWarsPlayerState->TeamId);
		}
	}

	return ActiveTeams.Num();
}

void AAccelByteWarsInGameGameMode::SpawnAndPossesPawn(APlayerState* PlayerState)
{
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
	NULLPTR_CHECK(ABPlayerState)

	AController* PlayerController = ABPlayerState->GetOwningController();
	NULLPTR_CHECK(PlayerController)

	// Only spawn if player have lives
	if (ABPlayerState->NumLivesLeft <= 0)
		return;

	// if team is not assigned, aka INDEX_NONE, do not spawn
	if (ABPlayerState->TeamId <= INDEX_NONE)
		return;

	// Pawn uses BP class pawn and "expose on spawn"ed parameters, use implementable event
	AAccelByteWarsPlayerPawn* Pawn = CreateShipPawn(FindGoodPlayerPosition(ABPlayerState), PlayerController);
	NULLPTR_CHECK(Pawn);

	// setup and posses
	SetupGameplayObject(Pawn);
	PlayerController->Possess(Pawn);
	Pawn->SetColor(ABPlayerState->TeamColor);

	// In case issue "missile go through object" happened again
	GAMEMODE_LOG(VeryVerbose, TEXT("Object spawned: %s | %f"), *Pawn->GetName(), Pawn->GetTransform().GetLocation().Z)
}

AAccelByteWarsPlayerPawn* AAccelByteWarsInGameGameMode::CreatePlayerPawn(
	const FVector& Location,
	APlayerController* PlayerController) const
{
	return CreateShipPawn(Location, PlayerController);
}

AAccelByteWarsPlayerPawn* AAccelByteWarsInGameGameMode::CreateShipPawn(const FVector& Location,
	AController* Controller) const
{
	if (Controller == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Controller;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Random rotation.
	const float RandomYaw = FMath::RandRange(180.0, -180.0);

	AAccelByteWarsPlayerPawn* NewPlayerPawn = Controller->GetWorld()->SpawnActor<AAccelByteWarsPlayerPawn>(
		PawnClass,
		FTransform(FRotator(0.0, RandomYaw, 0.0), Location),
		SpawnParameters);
	if (NewPlayerPawn == nullptr)
		return nullptr;

	UAccelByteWarsGameInstance* ABGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (ABGameInstance == nullptr)
		return nullptr;

	// Can be bot
	// AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	// if (ABPlayerController == nullptr)
	// 	return nullptr;

	APlayerState* PlayerState = Controller->PlayerState;
	if (PlayerState == nullptr)
		return nullptr;

	AAccelByteWarsPlayerState* ABPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (ABPlayerState == nullptr)
		return nullptr;

	NewPlayerPawn->SetReplicates(true);

	return NewPlayerPawn;
}

void AAccelByteWarsInGameGameMode::FillEmptySlotWithBot()
{
	const int32 CurrentPlayers = ABInGameGameState->PlayerArray.Num();
	const int32 MaxPlayers = ABInGameGameState->GameSetup.MaxPlayers;

	if (CurrentPlayers < MaxPlayers)
	{
		const int32 BotsNeeded = MaxPlayers - CurrentPlayers;

		for (int32 i = 0; i < BotsNeeded; ++i)
		{
			// Create bot controller
			AAccelByteWarsBotController* BotController = GetWorld()->SpawnActor<AAccelByteWarsBotController>(AAccelByteWarsBotController::StaticClass());
			BotController->InitPlayerState();
			ABInGameGameState->AddPlayerState(BotController->PlayerState);
			PlayerTeamSetup(BotController);
		}
	}
}

TArray<FVector> AAccelByteWarsInGameGameMode::GetActiveGameObjectsPosition() const
{
	TArray<FVector> Positions;

	for (const UAccelByteWarsGameplayObjectComponent* ActiveGameObject : ABInGameGameState->ActiveGameObjects)
	{
		// failsafe
		if (!ActiveGameObject)
		{
			continue;
		}

		const FVector& ActorLocation = ActiveGameObject->GetOwner()->GetActorLocation();
		const float Z = (ActiveGameObject->Radius * 100.0) + ObjectSafeDistance;
		Positions.Add(FVector(ActorLocation.X, ActorLocation.Y, Z));
	}

	return Positions;
}

bool AAccelByteWarsInGameGameMode::FindGoodSpawnLocation(FVector2D& OutCoord)
{
	if (!FindGoodSpawnLocation(
			OutCoord,
			GetActiveGameObjectsPosition(),
			ABInGameGameState->MinGameBound,
			ABInGameGameState->MaxGameBound))
	{
		return false;
	}
	return true;
}

void AAccelByteWarsInGameGameMode::SpawnPlanets()
{
	AccelByteWars2DProbabilityDistribution spawnGrid(ABInGameGameState->MinGameBound, ABInGameGameState->MaxGameBound, 20);

	int numRows = spawnGrid.GetNumRows();
	int numCols = spawnGrid.GetNumCols();
	FVector2D cellSize = spawnGrid.GetCellSize();

	spawnGrid.IterateGrid([numRows, numCols, cellSize](int col, int row, float& value) {
		value = std::min(
			{ (col + 1) * cellSize.X,
				(row + 1) * cellSize.Y,
				(numCols - col) * cellSize.X,
				(numRows - row) * cellSize.Y });
	});

	for (int i = 0; i < MaxTargetPlanetCount; ++i)
	{
		// random which planet to spawn
		const int32 RandomIndex = FMath::RandRange(0, PlanetMap.Num() - 1);
		const FPlanetMetadata PlanetData = *PlanetMap.Find(RandomIndex);

		// Calculate pseudorandom location
		FVector2D Location;
		if (!spawnGrid.FindGoodPosition(Location, PlanetData.PlanetRadius))
		{
			continue;
		}

		spawnGrid.IterateGrid([&spawnGrid, cellSize, Location, radius = PlanetData.PlanetRadius](int col, int row, float& value) {
			FVector2D cellPosition = spawnGrid.GetCellLocation(col, row);
			float distToCell = FVector2D::Distance(cellPosition, Location) - radius;
			distToCell = FMath::Max(0.0f, distToCell);
			value = FMath::Min(value, distToCell);
		});

		FVector Location3D = FVector(Location.X, Location.Y, 0.0f);
		const TSubclassOf<AActor>& ObjectToSpawn = ObjectsToSpawn[PlanetData.PlanetID];
		AActor* SpawnedObject = GetWorld()->SpawnActor(ObjectToSpawn, &Location3D, &FRotator::ZeroRotator);
		SetupGameplayObject(SpawnedObject);

		// In case issue "missile go through object" happened again
		GAMEMODE_LOG(VeryVerbose, TEXT("Object spawned: %s | %f"), *SpawnedObject->GetName(), SpawnedObject->GetTransform().GetLocation().Z)
	}
}

bool AAccelByteWarsInGameGameMode::FindGoodPlanetPosition(FVector& Position) const
{
	FVector2D MaxBound;
	FVector2D MinBound;

	// calculate allowable spawn area width and height
	const float PlanetSpawnProhibitedAreaDecimal = 1 - (PlanetSpawnAreaPercentage / 100.0f);
	const float ProhibitedWidth =
		FMath::Abs(ABInGameGameState->MaxGameBound.X - ABInGameGameState->MinGameBound.X) * PlanetSpawnProhibitedAreaDecimal;
	const float ProhibitedHeight =
		FMath::Abs(ABInGameGameState->MaxGameBound.Y - ABInGameGameState->MinGameBound.Y) * PlanetSpawnProhibitedAreaDecimal;

	// Set allowable planet spawn area
	MaxBound.X = ABInGameGameState->MaxGameBound.X - (ProhibitedWidth / 2);
	MaxBound.Y = ABInGameGameState->MaxGameBound.Y - (ProhibitedHeight / 2);
	MinBound.X = ABInGameGameState->MinGameBound.X + (ProhibitedWidth / 2);
	MinBound.Y = ABInGameGameState->MinGameBound.Y + (ProhibitedHeight / 2);

	// draw planets' spawn area
	if (bDrawBoundingBox)
	{
		const float BoundingBoxHalfWidth = FMath::Abs(MaxBound.X - MinBound.X) / 2;
		const float BoundingBoxHalfHeight = FMath::Abs(MaxBound.Y - MinBound.Y) / 2;
		const FVector BoxCenter =
			FVector(MinBound.X + BoundingBoxHalfWidth, MinBound.Y + BoundingBoxHalfHeight, 0.0f);
		const FVector BoxExtents =
			FVector(BoundingBoxHalfWidth, BoundingBoxHalfHeight, 100.0f); // Z extent is arbitrary for visibility
		DrawDebugBox(
			GetWorld(),
			BoxCenter,
			BoxExtents,
			FColor::Blue,
			true,
			-1.0f,
			0,
			10.0f);
	}

	const TArray<FVector> ActiveGameObjectsCoords = GetActiveGameObjectsPosition();
	FVector2D Position2D;
	if (!FindGoodSpawnLocation(
			Position2D,
			ActiveGameObjectsCoords,
			MinBound,
			MaxBound))
	{
		return false;
	}

	Position.X = Position2D.X;
	Position.Y = Position2D.Y;

	// Assign a constant Z coord in case the game assign a random number
	Position.Z = CommonZCoord;

	return true;
}

FVector AAccelByteWarsInGameGameMode::FindGoodPlayerPosition(APlayerState* PlayerState) const
{
	FVector Position = FVector::ZeroVector;
	FVector2D MaxBound;
	FVector2D MinBound;

	const AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
	if (ABPlayerState == nullptr)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerState is null, returning 0,0 as spawn location."));
		return Position;
	}

	// Quadrant based spawning, play area divided into 4 quadrants, and quadrant can only occupied by one ship
	const float HalfWidth = FMath::Abs(ABInGameGameState->MaxGameBound.X - ABInGameGameState->MinGameBound.X) / 2.0f;
	const float HalfHeight = FMath::Abs(ABInGameGameState->MaxGameBound.Y - ABInGameGameState->MinGameBound.Y) / 2.0f;

	// Quadrant balancing, make sure that the second player will be spawned diagonal to the first one
	constexpr int QuadrantIndices[4] = { 0, 3, 2, 1 };

	switch (QuadrantIndices[ABPlayerState->TeamId])
	{
		case 0: // Top-Left Quadrant
			MinBound.X = ABInGameGameState->MinGameBound.X;
			MinBound.Y = ABInGameGameState->MinGameBound.Y + HalfHeight + SafeZone;
			MaxBound.X = ABInGameGameState->MaxGameBound.X - HalfWidth - SafeZone;
			MaxBound.Y = ABInGameGameState->MaxGameBound.Y;
			break;
		case 1: // Top-Right Quadrant
			MinBound.X = ABInGameGameState->MinGameBound.X + HalfWidth + SafeZone;
			MinBound.Y = ABInGameGameState->MinGameBound.Y + HalfHeight + SafeZone;
			MaxBound.X = ABInGameGameState->MaxGameBound.X;
			MaxBound.Y = ABInGameGameState->MaxGameBound.Y;
			break;
		case 2: // Bottom-Left Quadrant
			MinBound.X = ABInGameGameState->MinGameBound.X;
			MinBound.Y = ABInGameGameState->MinGameBound.Y;
			MaxBound.X = ABInGameGameState->MaxGameBound.X - HalfWidth - SafeZone;
			MaxBound.Y = ABInGameGameState->MaxGameBound.Y - HalfHeight - SafeZone;
			break;
		case 3: // Bottom-Right Quadrant
			MinBound.X = ABInGameGameState->MinGameBound.X + HalfWidth + SafeZone;
			MinBound.Y = ABInGameGameState->MinGameBound.Y;
			MaxBound.X = ABInGameGameState->MaxGameBound.X;
			MaxBound.Y = ABInGameGameState->MaxGameBound.Y - HalfHeight - SafeZone;
			break;
		default:;
	}

	// pseudo-randomizer
	const TArray<FVector> ActiveGameObjectsCoords = GetActiveGameObjectsPosition();
	FVector2D Position2;
	if (!FindGoodSpawnLocation(Position2, ActiveGameObjectsCoords, MinBound, MaxBound))
	{
		GAMEMODE_LOG(Warning, TEXT("Can't find good spawn location for PLAYER. Please report"));
	}
	Position.X = Position2.X;
	Position.Y = Position2.Y;

	// Assign a constant Z coord in case the game assign a random number
	Position.Z = CommonZCoord;

	// Draw the spawn area box for debugging
	if (bDrawBoundingBox)
	{
		const float BoundingBoxHalfWidth = FMath::Abs(MaxBound.X - MinBound.X) / 2;
		const float BoundingBoxHalfHeight = FMath::Abs(MaxBound.Y - MinBound.Y) / 2;
		const FVector BoxCenter =
			FVector(MinBound.X + BoundingBoxHalfWidth, MinBound.Y + BoundingBoxHalfHeight, 0.0f);
		const FVector BoxExtents =
			FVector(BoundingBoxHalfWidth, BoundingBoxHalfHeight, 100.0f); // Z extent is arbitrary for visibility
		DrawDebugBox(
			GetWorld(),
			BoxCenter,
			BoxExtents,
			FColor::Green,
			true,
			-1.0f,
			0,
			10.0f);
	}

	return Position;
}
#pragma endregion

#pragma region "Countdown related"
// @@@SNIPSTART AccelByteWarsInGameGameMode.cpp-ShouldStartNotEnoughPlayerCountdown
bool AAccelByteWarsInGameGameMode::ShouldStartNotEnoughPlayerCountdown() const
{
	// check if the config is enabled in game setup
	if (ABInGameGameState->GameSetup.NotEnoughPlayerShutdownCountdown == INDEX_NONE || ABInGameGameState->GameSetup.MinimumTeamCountToPreventAutoShutdown == INDEX_NONE)
	{
		return false;
	}

	const int32 LivingTeamCount = GetLivingTeamCount();
	const int32 HumanPlayerCount = GetConnectedHumanPlayerCount();
	const int32 MinRequiredCount = ABInGameGameState->GameSetup.MinimumTeamCountToPreventAutoShutdown;

	// Start countdown if either not enough teams OR not enough actual human players
	return (LivingTeamCount < MinRequiredCount) || (HumanPlayerCount < MinRequiredCount);
}
// @@@SNIPEND

// @@@SNIPSTART AccelByteWarsInGameGameMode.cpp-NotEnoughPlayerCountdownCounting
void AAccelByteWarsInGameGameMode::NotEnoughPlayerCountdownCounting(const float& DeltaSeconds)
{
	// start NotEnoughPlayerCountdown to trigger server shutdown
	ABInGameGameState->NotEnoughPlayerCountdown -= DeltaSeconds;
	if (ABInGameGameState->NotEnoughPlayerCountdown <= 0)
	{
		ABInGameGameState->GameStatus = EGameStatus::INVALID;
		CloseGame("Not enough player");
	}
}
// @@@SNIPEND

void AAccelByteWarsInGameGameMode::SetupShutdownCountdownsValue() const
{
	ABInGameGameState->PostGameCountdown = ABInGameGameState->GameSetup.GameEndsShutdownCountdown;
	ABInGameGameState->NotEnoughPlayerCountdown = ABInGameGameState->GameSetup.NotEnoughPlayerShutdownCountdown;
}
#pragma endregion

#pragma region "Gameplay logic math helper"
bool AAccelByteWarsInGameGameMode::FindGoodSpawnLocation(
	FVector2D& OutCoord,
	const TArray<FVector>& ActiveGameObjectsCoords,
	const FVector2D& MinBound,
	const FVector2D& MaxBound) const
{
	AccelByteWars2DProbabilityDistribution spawnGrid(MinBound, MaxBound, 20);

	int numRows = spawnGrid.GetNumRows();
	int numCols = spawnGrid.GetNumCols();
	FVector2D cellSize = spawnGrid.GetCellSize();

	spawnGrid.IterateGrid([numRows, numCols, cellSize, &spawnGrid](int col, int row, float& value) {
		FVector2D cellLocation = spawnGrid.GetCellLocation(col, row);
		value = cellLocation.Length() - cellSize.X * 8.0f;
		// value = FMath::Min(value, FMath::Abs(cellLocation.X));
		// value = std::min<float>({ (float)col * (float)cellSize.X * 4.0f, (float)row * (float)cellSize.Y * 4.0f, value });
		value = FMath::Max(0.0f, value);
	});

	for (const FVector& CircleCoord : ActiveGameObjectsCoords)
	{
		FVector2D location(CircleCoord.X, CircleCoord.Y);
		spawnGrid.IterateGrid([&spawnGrid, cellSize, location, radius = CircleCoord.Z](int col, int row, float& value) {
			FVector2D cellPosition = spawnGrid.GetCellLocation(col, row);
			float distToCell = FVector2D::Distance(cellPosition, location);
			distToCell = FMath::Max(0.0f, distToCell);
			value = FMath::Min(value, distToCell);
		});
	}

	if (spawnGrid.FindGoodPosition(OutCoord, 250.0f))
	{
		return true;
	}

	if (spawnGrid.FindGoodPosition(OutCoord, 0.0f))
	{
		return true;
	}

	return false;
}

bool AAccelByteWarsInGameGameMode::IsInsideCircle(const FVector2D& Target, const FVector& Circle) const
{
	const double a = FMath::Pow(FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Target.X - Circle.X, 2.0), 0.5);
	return Target.Y < Circle.Y + a && Target.Y > Circle.Y - a;
}

double AAccelByteWarsInGameGameMode::CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord) const
{
	const double a = FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Ycoord - Circle.Y, 2.0);
	if (a < 0.0)
	{
		// This shouldn't happened if this is called only if IsInsideCircle return true. Just in case.
		return 0.0;
	}

	const double Xrel = FMath::Pow(a, 0.5);
	const double Xabs1 = Circle.X + Xrel;
	const double Xabs2 = Circle.X - Xrel;

	return FMath::Abs(Xabs1 - Xabs2);
}

FVector2D AAccelByteWarsInGameGameMode::CalculateActualCoord(
	const double RelativeLocation,
	const FVector2D& MinBound,
	const double RangeX) const
{
	FVector2D OutLocation;

	OutLocation.X = FMath::Fmod(RelativeLocation, RangeX) + MinBound.X;
	OutLocation.Y = (RelativeLocation / RangeX) + MinBound.Y;

	return OutLocation;
}

bool AAccelByteWarsInGameGameMode::LocationHasLineOfSightToOtherShip(const FVector& PositionToTest) const
{
	TArray<AActor*> PawnActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PawnActors);
	for (const AActor* PawnActor : PawnActors)
	{
		// check if pawn has line of sight to objects other than ship
		bool bHasLineOfSightOther = true;
		for (const UAccelByteWarsGameplayObjectComponent* Object : ABInGameGameState->ActiveGameObjects)
		{
			if (!Object)
			{
				continue;
			}

			if (Object->ObjectType == EGameplayObjectType::SHIP)
			{
				continue;
			}

			const float Radius = Object->Radius * 100.0f;
			const float DistanceToSegment = UKismetMathLibrary::GetPointDistanceToSegment(
				Object->GetOwner()->GetActorLocation(), PositionToTest, PawnActor->GetActorLocation());
			if (DistanceToSegment < Radius)
			{
				bHasLineOfSightOther = false;
				break;
			}
		}

		if (bHasLineOfSightOther)
		{
			return true;
		}
	}

	return false;
}

#pragma endregion

#pragma region "Debugging"
void AAccelByteWarsInGameGameMode::StartPlanetSpawningTimer(float InRate)
{
	GetWorldTimerManager().SetTimer(
		PlanetSpawningTimerHandle,
		this,
		&AAccelByteWarsInGameGameMode::ResetPlanets,
		InRate,
		true);
}

void AAccelByteWarsInGameGameMode::StopPlanetSpawningTimer()
{
	if (PlanetSpawningTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(PlanetSpawningTimerHandle);
	}
}

void AAccelByteWarsInGameGameMode::ResetPlanets()
{
	// remove planets
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		if (const UAccelByteWarsGameplayObjectComponent* Component =
				Cast<UAccelByteWarsGameplayObjectComponent>(Actor->GetComponentByClass(UAccelByteWarsGameplayObjectComponent::StaticClass())))
		{
			if (Component->ObjectType != EGameplayObjectType::SHIP)
			{
				Actor->Destroy();
			}
		}
	}

	// remove planets from game state
	for (UAccelByteWarsGameplayObjectComponent* Component : ABInGameGameState->ActiveGameObjects)
	{
		if (Component->ObjectType != EGameplayObjectType::SHIP)
		{
			ABInGameGameState->ActiveGameObjects.Remove(Component);
		}
	}

	SpawnPlanets();
}

void AAccelByteWarsInGameGameMode::SetPlanetTargetNum(const int32 NewTarget)
{
	MaxTargetPlanetCount = NewTarget;
}

void AAccelByteWarsInGameGameMode::SetObjectSafeDistance(float NewDistance)
{
	ObjectSafeDistance = NewDistance;
}

void AAccelByteWarsInGameGameMode::DrawBoundingBoxOnNextSpawn(const bool bDraw)
{
	bDrawBoundingBox = bDraw;
}

void AAccelByteWarsInGameGameMode::ModifyGameBoundExtendModifier(const float NewModifier) const
{
	const float NewHalfWidth =
		(FMath::Abs(ABInGameGameState->MaxGameBound.X - ABInGameGameState->MinGameBound.X) * (NewModifier - 1)) / 2;
	const float NewHalfHeight =
		(FMath::Abs(ABInGameGameState->MaxGameBound.Y - ABInGameGameState->MinGameBound.Y) * (NewModifier - 1)) / 2;
	ABInGameGameState->MaxGameBoundExtend = { ABInGameGameState->MaxGameBound.X + NewHalfWidth, ABInGameGameState->MaxGameBound.Y + NewHalfHeight };
	ABInGameGameState->MinGameBoundExtend = { ABInGameGameState->MinGameBound.X - NewHalfWidth, ABInGameGameState->MinGameBound.Y - NewHalfHeight };
}
#pragma endregion
