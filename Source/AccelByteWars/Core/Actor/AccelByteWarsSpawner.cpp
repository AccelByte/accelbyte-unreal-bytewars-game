// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsSpawner.h"
#include "AccelByteWars/Core/GameStates/AccelByteWarsInGameGameState.h"
#include "AccelByteWars/Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "AccelByteWarsAsteroid.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAccelByteWarsSpawner::AAccelByteWarsSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AAccelByteWarsSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	GameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(this));
	if (!GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("AccelByteWarsSpawner: Could not find AAccelByteWarsInGameGameState"));
		return;
	}

	// Initialize spawn counts for each actor type that appears in SpawnableActors
	for (const FSpawnableActorData& SpawnData : SpawnableActors)
	{
		if (!CurrentSpawnCounts.Contains(SpawnData.ActorType))
		{
			CurrentSpawnCounts.Add(SpawnData.ActorType, 0);
		}
	}
}

void AAccelByteWarsSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void AAccelByteWarsSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		CleanupDestroyedActors();
	}
}

void AAccelByteWarsSpawner::StartSpawning()
{
	if (!HasAuthority() || !bEnabled)
	{
		return;
	}
	bHasStartedMatchSpawning = true;

	// Clear existing timers and spawn counts
	CurrentSpawnCounts.Empty();
	SpawnTimers.Empty();

	// Initialize spawn counts and start spawning for each configured actor type
	for (const auto& SpawnTypeConfigPair : SpawnTypeConfig)
	{
		ESpawnableActorType ActorType = SpawnTypeConfigPair.Key;

		// Only initialize if we have actors of this type with positive weight
		if (GetTotalWeightForActorType(ActorType) > 0.0f)
		{
			CurrentSpawnCounts.Add(ActorType, 0);
			SpawnTimers.Add(ActorType, FTimerHandle());
			ScheduleNextSpawn(ActorType);
		}
	}
}

void AAccelByteWarsSpawner::StopSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	for (TPair<ESpawnableActorType, FTimerHandle>& SpawnTimerPair : SpawnTimers)
	{
		if (SpawnTimerPair.Value.IsValid())
		{
			GetWorldTimerManager().ClearTimer(SpawnTimerPair.Value);
		}
	}

	// Reset match spawning flag so it can restart if needed
	bHasStartedMatchSpawning = false;
}

void AAccelByteWarsSpawner::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;

	if (!bEnabled)
	{
		StopSpawning();
	}
	else if (bAutoStartSpawning)
	{
		StartSpawning();
	}
}

bool AAccelByteWarsSpawner::FindSpawnLocation(ESpawnZone Zone, FVector& OutLocation)
{
	switch (Zone)
	{
		case ESpawnZone::Edge:
			return FindEdgeSpawnLocation(OutLocation);
		case ESpawnZone::Random:
			return FMath::RandBool() ? FindCenterSpawnLocation(OutLocation) : FindEdgeSpawnLocation(OutLocation);
		case ESpawnZone::Center:
		default:
			return FindCenterSpawnLocation(OutLocation);
	}
}

void AAccelByteWarsSpawner::CleanupDestroyedActors()
{
	SpawnedActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
		return !ActorPtr.IsValid();
	});

	for (TPair<ESpawnableActorType, int32>& SpawnCount : CurrentSpawnCounts)
	{
		SpawnCount.Value = 0;
	}

	for (const TWeakObjectPtr<AActor>& ActorPtr : SpawnedActors)
	{
		if (!ActorPtr.IsValid())
		{
			continue;
		}
		for (const FSpawnableActorData& SpawnData : SpawnableActors)
		{
			if (ActorPtr->IsA(SpawnData.ActorClass))
			{
				CurrentSpawnCounts[SpawnData.ActorType]++;
				break;
			}
		}
	}
}

void AAccelByteWarsSpawner::ScheduleNextSpawn(ESpawnableActorType ActorType)
{
	if (!HasAuthority() || !bEnabled)
	{
		return;
	}

	if (!SpawnTypeConfig.Contains(ActorType))
	{
		return;
	}

	const FSpawnableConfig& SpawnConfig = SpawnTypeConfig[ActorType];

	float SpawnDelay = FMath::RandRange(SpawnConfig.MinSpawnInterval, SpawnConfig.MaxSpawnInterval);
	SpawnDelay *= GlobalSpawnRateMultiplier;

	FTimerDelegate SpawnDelegate;
	SpawnDelegate.BindUObject(this, &ThisClass::SpawnActorOfTypeScheduled, ActorType);

	FTimerHandle* TimerHandle = SpawnTimers.Find(ActorType);
	if (TimerHandle)
	{
		GetWorldTimerManager().SetTimer(
			*TimerHandle,
			SpawnDelegate,
			SpawnDelay,
			false);
	}
}

void AAccelByteWarsSpawner::SpawnActorOfTypeScheduled(ESpawnableActorType ActorType)
{
	if (!HasAuthority() || !bEnabled)
	{
		return;
	}

	if (!SpawnTypeConfig.Contains(ActorType))
	{
		return;
	}

	const FSpawnableConfig& SpawnConfig = SpawnTypeConfig[ActorType];

	int32 CurrentCount = CurrentSpawnCounts.FindRef(ActorType);
	if (CurrentCount >= SpawnConfig.MaxConcurrentSpawns)
	{
		ScheduleNextSpawn(ActorType);
		return;
	}

	// Use weighted selection to choose which actor to spawn of this type
	const int32 SelectedActorIndex = SelectRandomActorByWeight(ActorType);
	if (SelectedActorIndex == INDEX_NONE)
	{
		ScheduleNextSpawn(ActorType);
		return;
	}

	const FSpawnableActorData& SelectedSpawnData = SpawnableActors[SelectedActorIndex];

	// Spawn
	if (SelectedSpawnData.ActorClass)
	{
		FVector SpawnLocation;

		if (!FindSpawnLocation(SpawnConfig.PreferredSpawnZone, SpawnLocation))
		{
			ScheduleNextSpawn(ActorType);
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = this;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			SelectedSpawnData.ActorClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams);

		if (SpawnedActor)
		{
			SpawnedActors.Add(SpawnedActor);
			CurrentSpawnCounts[ActorType]++;

			if (ActorType == ESpawnableActorType::Asteroid)
			{
				// Randomize Asteroid properties before setting up movement
				RandomizeAsteroidProperties(SpawnedActor);

				FVector Direction = CalculateAsteroidDirection(SpawnLocation);
				SetupAsteroidMovement(SpawnedActor, Direction);
			}
		}
	}

	ScheduleNextSpawn(ActorType);
}

bool AAccelByteWarsSpawner::FindCenterSpawnLocation(FVector& OutLocation)
{
	if (!GameState)
	{
		return false;
	}

	// Use the same logic as GameMode's FindGoodPlanetPosition
	FVector2D MaxBound;
	FVector2D MinBound;

	// Calculate allowable spawn area width and height based on percentage
	const float SpawnProhibitedAreaDecimal = 1 - (SpawnerSettings.CenterSpawnAreaPercentage / 100.0f);
	const float ProhibitedWidth =
		FMath::Abs(GameState->MaxGameBound.X - GameState->MinGameBound.X) * SpawnProhibitedAreaDecimal;
	const float ProhibitedHeight =
		FMath::Abs(GameState->MaxGameBound.Y - GameState->MinGameBound.Y) * SpawnProhibitedAreaDecimal;

	// Set allowable spawn area
	MaxBound.X = GameState->MaxGameBound.X - (ProhibitedWidth / 2);
	MaxBound.Y = GameState->MaxGameBound.Y - (ProhibitedHeight / 2);
	MinBound.X = GameState->MinGameBound.X + (ProhibitedWidth / 2);
	MinBound.Y = GameState->MinGameBound.Y + (ProhibitedHeight / 2);

	// Use existing spawn location logic
	FVector2D OutCoord;
	TArray<FVector> ActiveObjectPositions = GetActiveObjectPositions();

	if (FindGoodSpawnLocationInternal(OutCoord, ActiveObjectPositions, MinBound, MaxBound))
	{
		OutLocation = FVector(OutCoord.X, OutCoord.Y, 0.0f);
		return true;
	}

	return false;
}

bool AAccelByteWarsSpawner::FindEdgeSpawnLocation(FVector& OutLocation)
{
	if (!GameState)
	{
		return false;
	}

	// Use extended bounds for edge spawning to ensure spawning outside max camera zoom
	OutLocation = GetRandomEdgeLocationOutsideCamera();
	return true;
}

FVector AAccelByteWarsSpawner::GetRandomEdgeLocation()
{
	if (!GameState)
	{
		return FVector::ZeroVector;
	}

	// Use GameState bounds for edge spawning
	FVector2D EdgeOffset = FVector2D(SpawnerSettings.EdgeZoneDistance, SpawnerSettings.EdgeZoneDistance);

	int32 EdgeSide = FMath::RandRange(0, 3);
	FVector EdgeLocation;

	switch (EdgeSide)
	{
		case 0: // Top
			EdgeLocation.X = FMath::RandRange(GameState->MinGameBound.X - EdgeOffset.X, GameState->MaxGameBound.X + EdgeOffset.X);
			EdgeLocation.Y = GameState->MaxGameBound.Y + EdgeOffset.Y;
			break;
		case 1: // Right
			EdgeLocation.X = GameState->MaxGameBound.X + EdgeOffset.X;
			EdgeLocation.Y = FMath::RandRange(GameState->MinGameBound.Y - EdgeOffset.Y, GameState->MaxGameBound.Y + EdgeOffset.Y);
			break;
		case 2: // Bottom
			EdgeLocation.X = FMath::RandRange(GameState->MinGameBound.X - EdgeOffset.X, GameState->MaxGameBound.X + EdgeOffset.X);
			EdgeLocation.Y = GameState->MinGameBound.Y - EdgeOffset.Y;
			break;
		case 3: // Left
			EdgeLocation.X = GameState->MinGameBound.X - EdgeOffset.X;
			EdgeLocation.Y = FMath::RandRange(GameState->MinGameBound.Y - EdgeOffset.Y, GameState->MaxGameBound.Y + EdgeOffset.Y);
			break;
	}

	EdgeLocation.Z = 0.0f;
	return EdgeLocation;
}

FVector AAccelByteWarsSpawner::CalculateAsteroidDirection(const FVector& EdgeLocation)
{
	if (!GameState)
	{
		return FVector::ForwardVector;
	}

	FVector CenterPoint = FVector(
		(GameState->MinGameBound.X + GameState->MaxGameBound.X) * 0.5f,
		(GameState->MinGameBound.Y + GameState->MaxGameBound.Y) * 0.5f,
		0.0f);

	FVector RandomTargetOffset = FVector(
		FMath::RandRange(-500.0f, 500.0f),
		FMath::RandRange(-500.0f, 500.0f),
		0.0f);

	FVector TargetPoint = CenterPoint + RandomTargetOffset;
	FVector Direction = (TargetPoint - EdgeLocation).GetSafeNormal();

	return Direction;
}

void AAccelByteWarsSpawner::SetupAsteroidMovement(AActor* Asteroid, const FVector& Direction)
{
	if (!Asteroid)
	{
		return;
	}
	FTimerHandle LifetimeTimer;

	GetWorldTimerManager().SetTimer(
		LifetimeTimer,
		FTimerDelegate::CreateWeakLambda(Asteroid, [Asteroid]() {
			Asteroid->Destroy();
		}),
		SpawnerSettings.AsteroidLifetime, false);
}

TArray<FVector> AAccelByteWarsSpawner::GetActiveObjectPositions()
{
	TArray<FVector> Positions;

	if (!GameState)
	{
		return Positions;
	}

	// Get positions and radii from active game objects (planets, ships, etc.)
	for (UAccelByteWarsGameplayObjectComponent* GameObject : GameState->ActiveGameObjects)
	{
		if (GameObject && GameObject->GetOwner())
		{
			FVector Location = GameObject->GetOwner()->GetActorLocation();
			// Store radius in Z component for collision calculations
			Location.Z = FMath::Max(GameObject->Radius, SpawnerSettings.SafeDistanceFromObjects);
			Positions.Add(Location);
		}
	}

	// Also include previously spawned actors with default safe distance
	for (const TWeakObjectPtr<AActor>& ActorPtr : SpawnedActors)
	{
		if (ActorPtr.IsValid())
		{
			FVector Location = ActorPtr->GetActorLocation();
			// Use default safe distance for spawned actors
			Location.Z = SpawnerSettings.SafeDistanceFromObjects;
			Positions.Add(Location);
		}
	}

	return Positions;
}

bool AAccelByteWarsSpawner::FindGoodSpawnLocationInternal(
	FVector2D& OutCoord,
	const TArray<FVector>& ActiveGameObjectsCoords,
	const FVector2D& MinBound,
	const FVector2D& MaxBound)
{
	// This mirrors the logic from AAccelByteWarsInGameGameMode::FindGoodSpawnLocation
	// Calculate prohibited area size
	double AreaProhibited = 0.0;
	for (const FVector& CircleCoord : ActiveGameObjectsCoords)
	{
		// Assuming all objects take the shape of a circle with Z as the radius
		// Closest distance between circle's center to box's outer rim
		float d = FMath::Sqrt(
			FMath::Square(FMath::Min(FMath::Max(CircleCoord.X, MinBound.X), MaxBound.X) - CircleCoord.X) + FMath::Square(FMath::Min(FMath::Max(CircleCoord.Y, MinBound.Y), MaxBound.Y) - CircleCoord.Y));
		float OverlapArea =
			FMath::Square(CircleCoord.Z) * FMath::Acos(d / (2 * CircleCoord.Z)) - ((d / 2) - FMath::Sqrt(FMath::Square(CircleCoord.Z) - FMath::Square(d / 2)));

		// If NaN, meaning the circle is entirely outside the bounding box
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
		UE_LOG(LogTemp, Warning, TEXT("AccelByteWarsSpawner: Not enough room, skipping spawn."));
		return false;
	}

	// Random value within the allowable area
	const double RandomRelativeLocation = FMath::RandRange(0.0, AreaAllowed);

	// Calculate the actual coordinate
	const double RangeX = Width;
	OutCoord = CalculateActualCoord(RandomRelativeLocation, MinBound, RangeX, ActiveGameObjectsCoords);

	return true;
}

FVector2D AAccelByteWarsSpawner::CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX, const TArray<FVector>& ActiveGameObjectsCoords) const
{
	// This mirrors the coordinate calculation logic from the GameMode
	double CurrentLocation = 0.0;
	const double RangeY = GameState->MaxGameBound.Y - GameState->MinGameBound.Y;

	for (double x = MinBound.X; x <= GameState->MaxGameBound.X; x += 1.0)
	{
		for (double y = MinBound.Y; y <= GameState->MaxGameBound.Y; y += 1.0)
		{
			// Check if this location is inside any prohibited circle
			bool bInsideProhibitedArea = false;
			for (const FVector& CircleCoord : ActiveGameObjectsCoords)
			{
				if (IsInsideCircle(FVector2D(x, y), CircleCoord))
				{
					bInsideProhibitedArea = true;
					break;
				}
			}

			if (!bInsideProhibitedArea)
			{
				CurrentLocation += 1.0;
				if (CurrentLocation >= RelativeLocation)
				{
					return FVector2D(x, y);
				}
			}
		}
	}

	// Fallback to center if calculation fails
	return FVector2D(
		(MinBound.X + GameState->MaxGameBound.X) * 0.5,
		(MinBound.Y + GameState->MaxGameBound.Y) * 0.5);
}

bool AAccelByteWarsSpawner::IsInsideCircle(const FVector2D& Target, const FVector& Circle) const
{
	const double Distance = FVector2D::Distance(Target, FVector2D(Circle.X, Circle.Y));
	return Distance <= Circle.Z;
}

void AAccelByteWarsSpawner::RandomizeAsteroidProperties(AActor* Asteroid)
{
	AAccelByteWarsAsteroid* AsteroidActor = Cast<AAccelByteWarsAsteroid>(Asteroid);
	if (!AsteroidActor)
	{
		return;
	}

	// Get current properties to modify
	FAsteroidProperties NewProperties = AsteroidActor->Properties;

	// Randomize rotation speed
	if (AsteroidRandomization.bRandomizeRotationSpeed)
	{
		NewProperties.RotationSpeed = FMath::RandRange(AsteroidRandomization.MinRotationSpeed, AsteroidRandomization.MaxRotationSpeed);
	}

	// Apply the randomized properties
	AsteroidActor->SetAsteroidProperties(NewProperties);
}

FVector AAccelByteWarsSpawner::GetRandomEdgeLocationOutsideCamera()
{
	if (!GameState)
	{
		return FVector::ZeroVector;
	}

	// Use extended game bounds to ensure spawning outside maximum camera zoom
	// These bounds represent the maximum area the camera can zoom out to
	FVector2D MaxBound = GameState->MaxGameBoundExtend;
	FVector2D MinBound = GameState->MinGameBoundExtend;

	// Add additional edge zone distance to ensure spawning outside camera view
	FVector2D EdgeOffset = FVector2D(SpawnerSettings.EdgeZoneDistance, SpawnerSettings.EdgeZoneDistance);

	// Expand bounds to be outside the maximum camera zoom area
	FVector2D SpawnMinBound = MinBound - EdgeOffset;
	FVector2D SpawnMaxBound = MaxBound + EdgeOffset;

	// Choose random edge side
	int32 EdgeSide = FMath::RandRange(0, 3);
	FVector EdgeLocation;

	switch (EdgeSide)
	{
		case 0: // Top
			EdgeLocation.X = FMath::RandRange(SpawnMinBound.X, SpawnMaxBound.X);
			EdgeLocation.Y = SpawnMaxBound.Y;
			break;
		case 1: // Right
			EdgeLocation.X = SpawnMaxBound.X;
			EdgeLocation.Y = FMath::RandRange(SpawnMinBound.Y, SpawnMaxBound.Y);
			break;
		case 2: // Bottom
			EdgeLocation.X = FMath::RandRange(SpawnMinBound.X, SpawnMaxBound.X);
			EdgeLocation.Y = SpawnMinBound.Y;
			break;
		case 3: // Left
			EdgeLocation.X = SpawnMinBound.X;
			EdgeLocation.Y = FMath::RandRange(SpawnMinBound.Y, SpawnMaxBound.Y);
			break;
	}

	EdgeLocation.Z = 0.0f;
	return EdgeLocation;
}

int32 AAccelByteWarsSpawner::SelectRandomActorByWeight(ESpawnableActorType ActorType) const
{
	TArray<int32> ValidIndices;
	TArray<float> Weights;

	// Gather all valid actors of the specified type with their weights
	for (int32 i = 0; i < SpawnableActors.Num(); i++)
	{
		if (SpawnableActors[i].ActorType == ActorType && SpawnableActors[i].Weight > 0.0f)
		{
			ValidIndices.Add(i);
			Weights.Add(SpawnableActors[i].Weight);
		}
	}

	if (ValidIndices.Num() == 0)
	{
		return INDEX_NONE;
	}

	// If only one valid actor, return it
	if (ValidIndices.Num() == 1)
	{
		return ValidIndices[0];
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return INDEX_NONE;
	}

	// Generate random number between 0 and total weight
	float RandomValue = FMath::RandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	// Find the selected actor based on weight
	for (int32 i = 0; i < ValidIndices.Num(); i++)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return ValidIndices[i];
		}
	}

	// Fallback to last valid index if calculation fails
	return ValidIndices.Last();
}

float AAccelByteWarsSpawner::GetTotalWeightForActorType(ESpawnableActorType ActorType) const
{
	float TotalWeight = 0.0f;

	for (const FSpawnableActorData& SpawnData : SpawnableActors)
	{
		if (SpawnData.ActorType == ActorType)
		{
			TotalWeight += SpawnData.Weight;
		}
	}

	return TotalWeight;
}