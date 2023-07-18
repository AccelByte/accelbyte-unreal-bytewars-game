// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/GameModes/AccelByteWarsInGameGameMode.h"

#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define NULLPTR_CHECK(Object) if (!Object) return;

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
			if (CheckIfAllPlayersIsInOneTeam())
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
			if (CheckIfAllPlayersIsInOneTeam())
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
		if (IsRunningDedicatedServer())
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

	// Setup player if game has started
	if (ABInGameGameState->HasGameStarted() && HasMatchStarted())
	{
		SpawnAndPossesPawn(NewPlayer->PlayerState);
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
		ABInGameGameState->GetPlayerDataById(PlayerState->GetUniqueId(),GetControllerId(PlayerState));
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
		ABInGameGameState->GetPlayerDataById(PlayerState->GetUniqueId(),GetControllerId(PlayerState));
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

	// FX logic
	OnShipDestroyedFX(SourcePlayerController, ShipOwner->GetTransform(), ShipPlayerState);

	// Adjust player's var
	DecreasePlayerLife(ShipPlayerState, 1);
	// If friendly, do nothing
	if (SourcePlayerState->TeamId != ShipPlayerState->TeamId)
	{
		AddPlayerScore(SourcePlayerState, MissileScore);
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
	if (CheckIfAllPlayersIsInOneTeam())
	{
		EndGame("One team remains");
		return;
	}

	// If score limit reached, end game
	FGameplayTeamData TeamData;
	float TeamScore;
	int32 TeamLivesLeft;
	int32 TeamKillCount;
	ABInGameGameState->GetTeamDataByTeamId(SourcePlayerState->TeamId, TeamData, TeamScore, TeamLivesLeft, TeamKillCount);
	if (ABInGameGameState->GameSetup.ScoreLimit >= 0)
	{
		if (TeamScore >= ABInGameGameState->GameSetup.ScoreLimit)
		{
			EndGame("Score limit reached");
			return;
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
void AAccelByteWarsInGameGameMode::CloseGame(const FString& Reason) const
{
	GAMEMODE_LOG(Warning, TEXT("Unregistering or shutting down server with reason: %s."), *Reason);

	AAccelByteWarsGameSession* Session = Cast<AAccelByteWarsGameSession>(GameSession);
	if (!Session)
	{
		GAMEMODE_LOG(Warning, TEXT("Cannot unregister server, the game session is null. Shutting down immediately."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	// Unregister the server.
	Session->UnregisterServer();
}

void AAccelByteWarsInGameGameMode::StartGame()
{
	const FRotator Rotator = FRotator::ZeroRotator;

	// Spawn player start
	for (const FVector& PlayerStart : PlayerStartPoints)
	{
		GetWorld()->SpawnActor<APlayerStart>(PlayerStart, Rotator);
	}

	// Spawn objects
	const int MaxObjectNum = UKismetMathLibrary::RandomIntegerInRange(2, 5);
	for (int i = 0; i <= MaxObjectNum; ++i)
	{
		const TSubclassOf<AActor>& ObjectToSpawn =
			ObjectsToSpawn[UKismetMathLibrary::RandomIntegerInRange(0, ObjectsToSpawn.Num() - 1)];

		const FVector& Location = FindGoodPlanetPosition();
		AActor* SpawnedObject = GetWorld()->SpawnActor(ObjectToSpawn, &Location, &Rotator);

		SetupGameplayObject(SpawnedObject);
	}

	// Spawn and posses pawn for existing players
	for (APlayerState* PlayerState : ABInGameGameState->PlayerArray)
	{
		SpawnAndPossesPawn(PlayerState);
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

bool AAccelByteWarsInGameGameMode::CheckIfAllPlayersIsInOneTeam() const
{
	// check currently connected player TeamId
	int32 TeamId = INDEX_NONE;
	bool bOneTeamConnected = true;

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

	return bOneTeamConnected;
}

void AAccelByteWarsInGameGameMode::SpawnAndPossesPawn(APlayerState* PlayerState)
{
	if (const AAccelByteWarsPlayerState* PS = Cast<AAccelByteWarsPlayerState>(PlayerState))
	{
		// Only spawn if player have lives
		if (PS->NumLivesLeft <= 0)
		{
			return;
		}

		if (APlayerController* PC = PS->GetPlayerController())
		{
			// Pawn uses BP class pawn and "expose on spawn"ed parameters, use implementable event
			APawn* Pawn = CreatePlayerPawn(FindGoodPlayerPosition(), PS->TeamColor, PC);
			NULLPTR_CHECK(Pawn);

			// setup and posses
			SetupGameplayObject(Pawn);
			PC->Possess(Pawn);
		}
	}
}

TArray<FVector> AAccelByteWarsInGameGameMode::GetActiveGameObjectsPosition(
	const float SeparationFactor,
	const float ShipSeparationFactor) const
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
		const float Z = ActiveGameObject->Radius * 100.0 *
			(ActiveGameObject->ObjectType == EGameplayObjectType::SHIP ? ShipSeparationFactor : SeparationFactor);
		Positions.Add(FVector(ActorLocation.X, ActorLocation.Y, Z));
	}

	return Positions;
}

FVector AAccelByteWarsInGameGameMode::FindGoodPlanetPosition() const
{
	FVector Position;

	const FVector2D Position2 = FindGoodSpawnLocation(GetActiveGameObjectsPosition(4.0f, 4.0f));
	Position.X = Position2.X;
	Position.Y = Position2.Y;

	return Position;
}

FVector AAccelByteWarsInGameGameMode::FindGoodPlayerPosition() const
{
	FVector Position;

	const TArray<FVector> ActiveGameObjectLocations = GetActiveGameObjectsPosition(1.5f, 25.0f);

	// attempt to find location without line of sight. If failed, use the last attempt
	for (int i = 0; i <= 7; ++i)
	{
		const FVector2D Position2 = FindGoodSpawnLocation(ActiveGameObjectLocations);
		Position.X = Position2.X;
		Position.Y = Position2.Y;

		if (!LocationHasLineOfSightToOtherShip(Position))
		{
			break;
		}
	}

	return Position;
}
#pragma endregion 

#pragma region "Countdown related"
void AAccelByteWarsInGameGameMode::NotEnoughPlayerCountdownCounting(const float& DeltaSeconds) const
{
	// start NotEnoughPlayerCountdown to trigger server shutdown
	ABInGameGameState->NotEnoughPlayerCountdown -= DeltaSeconds;
	if (ABInGameGameState->NotEnoughPlayerCountdown <= 0)
	{
		CloseGame("Not enough player");
	}
}

void AAccelByteWarsInGameGameMode::SetupShutdownCountdownsValue() const
{
	// get specified ShutdownOnFinishedDelay timer value
	FString ShutdownOnFinishedDelayString = "30";
	FParse::Value(FCommandLine::Get(), TEXT("-ShutdownOnFinishedDelay="), ShutdownOnFinishedDelayString);
	ABInGameGameState->PostGameCountdown = FCString::Atoi(*ShutdownOnFinishedDelayString);

	// get specified ShutdownOnOneTeamOrLessDelay timer value
	FString ShutdownOnOneTeamOrLessDelayString = "30";
	FParse::Value(FCommandLine::Get(), TEXT("-ShutdownOnOneTeamOrLessDelay="), ShutdownOnOneTeamOrLessDelayString);
	ABInGameGameState->NotEnoughPlayerCountdown = FCString::Atoi(*ShutdownOnOneTeamOrLessDelayString);
}
#pragma endregion

#pragma region "Gameplay logic math helper"
FVector2D AAccelByteWarsInGameGameMode::FindGoodSpawnLocation(const TArray<FVector>& ActiveGameObjectsCoords) const
{
	FVector2D& MaxBound = ABInGameGameState->MaxGameBound;
	const FVector2D& MinBound = ABInGameGameState->MinGameBound;

	// Calculate prohibited area size
	double AreaProhibited = 0.0;
	for (const FVector& Coord : ActiveGameObjectsCoords)
	{
		// assuming all object takes the shape of a circle with Z as the radius
		AreaProhibited += UKismetMathLibrary::Abs(UKismetMathLibrary::GetPI() * FMath::Pow(Coord.Z, 2.0));
	}

	// Calculate total play area size
	double RangeX = (MaxBound.X - MinBound.X);
	double RangeY = (MaxBound.Y - MinBound.Y);
	double AreaTotal = UKismetMathLibrary::Abs(RangeX * RangeY);

	// Calculate allowable area size
	double AreaAllowed = AreaTotal - AreaProhibited;

	// Negative value means not enough room in the game play area -> increase play area while maintaining the original ratio
	if (AreaAllowed < 0)
	{
		const double n = FMath::Pow((AreaTotal + FMath::Abs(AreaAllowed)) / AreaTotal, 0.5);
		RangeX *= n;
		RangeY *= n;

		MaxBound.X = (RangeX + MinBound.X);
		MaxBound.Y = (RangeY + MinBound.Y);

		AreaTotal += UKismetMathLibrary::Abs(RangeX * RangeY);
		AreaAllowed = AreaTotal - AreaProhibited;

		UE_LOG(LogTemp, Log, TEXT("Play area is smaller than the prohibited area. Increasing play area. New MaxBound: %s"), *MaxBound.ToString());
	}

	// Random value within the allowable area
	double RelativeLocation = UKismetMathLibrary::RandomFloatInRange(0.0f, AreaAllowed);

	FVector2D OutLocation = CalculateActualCoord(RelativeLocation, MinBound, RangeX);

	// check if "overlapped" with existing objects
	bool bUpdated = true;
	while (bUpdated)
	{
		bUpdated = false;
		for (const FVector& Coord : ActiveGameObjectsCoords)
		{
			if (IsInsideCircle(OutLocation, Coord))
			{
				// Add the X axis length inside the circle on the specified Y to the RelativeLocation
				RelativeLocation += CalculateCircleLengthAlongXonY(Coord, OutLocation.Y);
				OutLocation = CalculateActualCoord(RelativeLocation, MinBound, RangeX);

				bUpdated = true;
				break;
			}
		}
	}

	return OutLocation;
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
