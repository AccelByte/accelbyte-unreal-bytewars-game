// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/GameModes/AccelByteWarsInGameGameMode.h"

#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define NULLPTR_CHECK(Object) if (!Object) return;

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

void AAccelByteWarsInGameGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (ABInGameGameState->GameStatus)
	{
	case EGameStatus::IDLE:
	case EGameStatus::AWAITING_PLAYERS:
		// check if all registered players have re-enter the server
		if (ABInGameGameState->PlayerArray.Num() == ABInGameGameState->GetRegisteredPlayersNum())
		{
			ABInGameGameState->GameStatus = EGameStatus::PRE_GAME_COUNTDOWN_STARTED;
			if (IsRunningDedicatedServer())
			{
				// reset NotEnoughPlayerCountdown
				SetupShutdownCountdownsValue();
			}
		}
		else
		{
			// use NotEnoughPlayerCountdown as a countdown to wait all registered player to reconnect to the DS
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
			EndGame("Time is over");
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
	default: ;
	}
}

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
		ABInGameGameState->HasGameStarted() &&
		HasMatchStarted() &&
		!AbPlayerState->bPendingTeamAssignment &&
		!ABInGameGameState->HasGameEnded())
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

void AAccelByteWarsInGameGameMode::EndGame(const FString Reason)
{
	ABInGameGameState->GameStatus = EGameStatus::GAME_ENDS_DELAY;

	OnGameEndsDelegate.Broadcast();

	GAMEMODE_LOG(Log, TEXT("Game ends with reason: %s."), *Reason);
}

void AAccelByteWarsInGameGameMode::OnShipDestroyed(
	UAccelByteWarsGameplayObjectComponent* Ship,
	const float MissileScore,
	APlayerController* SourcePlayerController)
{
	NULLPTR_CHECK(Ship);
	NULLPTR_CHECK(SourcePlayerController);

	AAccelByteWarsPlayerState* SourcePlayerState = Cast<AAccelByteWarsPlayerState>(SourcePlayerController->PlayerState);
	NULLPTR_CHECK(SourcePlayerState);

	const AActor* ShipActor = Ship->GetOwner();
	NULLPTR_CHECK(ShipActor);

	const APawn* ShipOwner = Cast<APawn>(ShipActor->GetNetOwner());
	NULLPTR_CHECK(ShipOwner);

	const APlayerController* ShipPC = Cast<APlayerController>(ShipOwner->Controller);
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
	if (SourcePlayerState->TeamId != ShipPlayerState->TeamId)
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
		EndGame("One team remains");
		return;
	}

	// If score limit reached, end game
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

void AAccelByteWarsInGameGameMode::IncreasePlayerKilledAttempt(const APlayerController* TargetPlayer)
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

#pragma region "Gameplay logic"
void AAccelByteWarsInGameGameMode::StartGame()
{
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

	// Spawn planets
	SpawnPlanets();
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
	for (const TObjectPtr<APlayerState> Player : ABInGameGameState->PlayerArray)
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

	APlayerController* PlayerController = ABPlayerState->GetPlayerController();
	NULLPTR_CHECK(PlayerController)

	// Only spawn if player have lives
	if (ABPlayerState->NumLivesLeft <= 0)
		return;

	// if team is not assigned, aka INDEX_NONE, do not spawn
	if (ABPlayerState->TeamId <= INDEX_NONE)
		return;

	// Pawn uses BP class pawn and "expose on spawn"ed parameters, use implementable event
	AAccelByteWarsPlayerPawn* Pawn = CreatePlayerPawn(FindGoodPlayerPosition(ABPlayerState), PlayerController);
	NULLPTR_CHECK(Pawn);

	// setup and posses
	SetupGameplayObject(Pawn);
	PlayerController->Possess(Pawn);
	Pawn->SetColor(ABPlayerState->TeamColor);
}

AAccelByteWarsPlayerPawn* AAccelByteWarsInGameGameMode::CreatePlayerPawn(
	const FVector& Location,
	APlayerController* PlayerController) const
{
	if (PlayerController == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerController;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAccelByteWarsPlayerPawn* NewPlayerPawn = PlayerController->GetWorld()->SpawnActor<AAccelByteWarsPlayerPawn>(
		PawnClass,
		FTransform(FRotator::ZeroRotator, Location),
		SpawnParameters);
	if (NewPlayerPawn == nullptr)
		return nullptr;

	UAccelByteWarsGameInstance* ABGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (ABGameInstance == nullptr)
		return nullptr;

	AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (ABPlayerController == nullptr)
		return nullptr;

	APlayerState* PlayerState = ABPlayerController->PlayerState;
	if (PlayerState == nullptr)
		return nullptr;

	AAccelByteWarsPlayerState* ABPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (ABPlayerState == nullptr)
		return nullptr;

	NewPlayerPawn->SetReplicates(true);

	return NewPlayerPawn;
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
	for (int i = 0; i < MaxTargetPlanetCount; ++i)
	{
		// random which planet to spawn
		const int32 RandomIndex = FMath::RandRange(0, PlanetMap.Num() - 1);
		const FPlanetMetadata PlanetData = *PlanetMap.Find(RandomIndex);

		// Calculate pseudorandom location
		FVector Location;
		if (!FindGoodPlanetPosition(Location))
		{
			continue;
		}

		const TSubclassOf<AActor>& ObjectToSpawn = ObjectsToSpawn[PlanetData.PlanetID];
		AActor* SpawnedObject = GetWorld()->SpawnActor(ObjectToSpawn, &Location, &FRotator::ZeroRotator);
		SetupGameplayObject(SpawnedObject);
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
	constexpr int QuadrantIndices[4] = {0, 3, 2, 1};

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
	default: ;
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
bool AAccelByteWarsInGameGameMode::ShouldStartNotEnoughPlayerCountdown() const
{
	// check if the config is enabled in game setup
	if (ABInGameGameState->GameSetup.NotEnoughPlayerShutdownCountdown == INDEX_NONE ||
		ABInGameGameState->GameSetup.MinimumTeamCountToPreventAutoShutdown == INDEX_NONE)
	{
		return false;
	}

	return GetLivingTeamCount() < ABInGameGameState->GameSetup.MinimumTeamCountToPreventAutoShutdown;
}

void AAccelByteWarsInGameGameMode::NotEnoughPlayerCountdownCounting(const float& DeltaSeconds) const
{
	// start NotEnoughPlayerCountdown to trigger server shutdown
	ABInGameGameState->NotEnoughPlayerCountdown -= DeltaSeconds;
	if (ABInGameGameState->NotEnoughPlayerCountdown <= 0)
	{
		ABInGameGameState->GameStatus = EGameStatus::INVALID;
		CloseGame("Not enough player");
	}
}

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
	// Calculate prohibited area size
	double AreaProhibited = 0.0;
	for (const FVector& CircleCoord : ActiveGameObjectsCoords)
	{
		// assuming all object takes the shape of a circle with Z as the radius
		// closest distance between circle's center to box's outer rim
		float d = FMath::Sqrt(
			FMath::Square(FMath::Min(FMath::Max(CircleCoord.X, MinBound.X), MaxBound.X) - CircleCoord.X) +
			FMath::Square(FMath::Min(FMath::Max(CircleCoord.Y, MinBound.Y), MaxBound.Y) - CircleCoord.Y));
		float OverlapArea =
			FMath::Square(CircleCoord.Z) *
			FMath::Acos(d / (2 * CircleCoord.Z)) -
			((d / 2) - FMath::Sqrt(
				FMath::Square(CircleCoord.Z) -
				FMath::Square(d / 2)));

		// if NaN, meaning the circle is entirely outside the bounding box
		if (FMath::IsNaN(OverlapArea))
		{
			OverlapArea = 0;
		}

		AreaProhibited += OverlapArea;
	}

	// Calculate total play area size
	const double Width = (MaxBound.X - MinBound.X);
	const double Height = (MaxBound.Y - MinBound.Y);
	const double AreaTotal = UKismetMathLibrary::Abs(Width * Height);

	// Calculate allowable area size
	const double AreaAllowed = AreaTotal - AreaProhibited;

	// Negative value means not enough room in the game play area -> skip spawn
	if (AreaAllowed < 0)
	{
		GAMEMODE_LOG(Warning, TEXT("Not enough room, skipping spawn."));
		return false;
	}

	// Random value within the allowable area
	double RelativeLocation = static_cast<float>(UKismetMathLibrary::RandomIntegerInRange(0, AreaAllowed));

	OutCoord = CalculateActualCoord(RelativeLocation, MinBound, Width);

	// check if "overlapped" with existing objects
	bool bUpdated = true;
	while (bUpdated)
	{
		bUpdated = false;
		for (const FVector& CircleCoord : ActiveGameObjectsCoords)
		{
			if (IsInsideCircle(OutCoord, CircleCoord))
			{
				// Add the X axis length inside the circle on the specified Y to the RelativeLocation
				RelativeLocation += CalculateCircleLengthAlongXonY(CircleCoord, OutCoord.Y);
				OutCoord = CalculateActualCoord(RelativeLocation, MinBound, Width);

				bUpdated = true;
				break;
			}
		}
	}

	return true;
}

bool AAccelByteWarsInGameGameMode::IsInsideCircle(const FVector2D& Target, const FVector& Circle) const
{
	const double a = FMath::Pow(FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Target.X - Circle.X, 2.0), 0.5);
	return
		Target.Y < Circle.Y + a && Target.Y > Circle.Y - a;
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
	ABInGameGameState->MaxGameBoundExtend =
		{ABInGameGameState->MaxGameBound.X + NewHalfWidth, ABInGameGameState->MaxGameBound.Y + NewHalfHeight};
	ABInGameGameState->MinGameBoundExtend =
		{ABInGameGameState->MinGameBound.X - NewHalfWidth, ABInGameGameState->MinGameBound.Y - NewHalfHeight};
}
#pragma endregion 
