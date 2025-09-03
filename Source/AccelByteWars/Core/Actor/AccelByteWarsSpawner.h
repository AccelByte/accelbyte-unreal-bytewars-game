// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "AccelByteWarsSpawner.generated.h"

class AAccelByteWarsInGameGameState;

UENUM(BlueprintType)
enum class ESpawnableActorType : uint8
{
	Crate = 0,
	Asteroid = 1
};

UENUM(BlueprintType)
enum class ESpawnZone : uint8
{
	Center = 0, // Center area of the map
	Edge = 1,	// Edge of the map (outside screen bounds)
	Random = 2	// Randomly choose between center and edge
};

USTRUCT(BlueprintType)
struct FSpawnableActorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	ESpawnableActorType ActorType = ESpawnableActorType::Crate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FSpawnableConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	ESpawnZone PreferredSpawnZone = ESpawnZone::Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	float MinSpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	float MaxSpawnInterval = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
	int32 MaxConcurrentSpawns = 3;
};

USTRUCT(BlueprintType)
struct FAsteroidRandomizationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Randomization")
	bool bRandomizeRotationSpeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Randomization", meta = (EditCondition = "bRandomizeRotationSpeed"))
	float MinRotationSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Randomization", meta = (EditCondition = "bRandomizeRotationSpeed"))
	float MaxRotationSpeed = 80.0f;
};

USTRUCT(BlueprintType)
struct FSpawnerSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	float CenterSpawnAreaPercentage = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	float EdgeZoneDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	float SafeDistanceFromObjects = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	float AsteroidLifetime = 20.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API AAccelByteWarsSpawner : public AActor
{
	GENERATED_BODY()

public:
	AAccelByteWarsSpawner();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	TArray<FSpawnableActorData> SpawnableActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	TMap<ESpawnableActorType, FSpawnableConfig> SpawnTypeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	FSpawnerSettings SpawnerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Settings")
	FAsteroidRandomizationSettings AsteroidRandomization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	bool bAutoStartSpawning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Settings")
	float GlobalSpawnRateMultiplier = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	bool FindSpawnLocation(ESpawnZone Zone, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void CleanupDestroyedActors();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ScheduleNextSpawn(ESpawnableActorType ActorType);
	void SpawnActorOfTypeScheduled(ESpawnableActorType ActorType);
	int32 SelectRandomActorByWeight(ESpawnableActorType ActorType) const;
	float GetTotalWeightForActorType(ESpawnableActorType ActorType) const;

	bool FindCenterSpawnLocation(FVector& OutLocation);
	bool FindEdgeSpawnLocation(FVector& OutLocation);
	FVector GetRandomEdgeLocation();
	FVector GetRandomEdgeLocationOutsideCamera();
	FVector CalculateAsteroidDirection(const FVector& EdgeLocation);
	void SetupAsteroidMovement(AActor* Asteroid, const FVector& Direction);
	void RandomizeAsteroidProperties(AActor* Asteroid);

	TArray<FVector> GetActiveObjectPositions();

	bool FindGoodSpawnLocationInternal(
		FVector2D& OutCoord,
		const TArray<FVector>& ActiveGameObjectsCoords,
		const FVector2D& MinBound,
		const FVector2D& MaxBound);

	FVector2D CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX, const TArray<FVector>& ActiveGameObjectsCoords) const;
	bool IsInsideCircle(const FVector2D& Target, const FVector& Circle) const;

	UPROPERTY()
	AAccelByteWarsInGameGameState* GameState = nullptr;

	UPROPERTY()
	TMap<ESpawnableActorType, FTimerHandle> SpawnTimers;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedActors;

	TMap<ESpawnableActorType, int32> CurrentSpawnCounts;

	bool bHasStartedMatchSpawning = false;
};