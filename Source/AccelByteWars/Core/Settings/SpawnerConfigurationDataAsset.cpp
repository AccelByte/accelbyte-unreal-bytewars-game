// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SpawnerConfigurationDataAsset.h"
#include "Core/Actor/AccelByteWarsAsteroid.h"

USpawnerConfigurationDataAsset::USpawnerConfigurationDataAsset()
{
	// Set up default configuration for Asteroids
	Configuration = FSpawnerConfiguration();

	// Create default Asteroid spawnable actor data
	FSpawnableActorData DefaultAsteroidData;
	DefaultAsteroidData.ActorClass = AAccelByteWarsAsteroid::StaticClass();
	DefaultAsteroidData.ActorType = ESpawnableActorType::Asteroid;

	Configuration.SpawnableActors.Add(DefaultAsteroidData);

	// Set up default spawner settings
	Configuration.SpawnerSettings.CenterSpawnAreaPercentage = 70.0f;
	Configuration.SpawnerSettings.EdgeZoneDistance = 300.0f;
	Configuration.SpawnerSettings.SafeDistanceFromObjects = 300.0f;
	Configuration.SpawnerSettings.AsteroidLifetime = 20.0f;

	// Set up default Asteroid randomization
	Configuration.AsteroidRandomization.bRandomizeRotationSpeed = true;
	Configuration.AsteroidRandomization.MinRotationSpeed = 20.0f;
	Configuration.AsteroidRandomization.MaxRotationSpeed = 80.0f;

	// Set up default spawner behavior
	Configuration.bAutoStartSpawning = true;
	Configuration.bEnabled = true;
	Configuration.GlobalSpawnRateMultiplier = 1.0f;
}

USpawnerConfigurationDataAsset* USpawnerConfigurationDataAsset::GetByCodeName(const FString& InCodeName)
{
	FPrimaryAssetId AssetId = FPrimaryAssetId(USpawnerConfigurationDataAsset::StaticClass()->GetFName(), FName(*InCodeName));
	return Cast<USpawnerConfigurationDataAsset>(UAccelByteWarsAssetManager::Get().GetPrimaryAssetObject(AssetId));
}

void USpawnerConfigurationDataAsset::ApplyConfigurationToSpawner(AAccelByteWarsSpawner* Spawner) const
{
	if (!Spawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("USpawnerConfigurationDataAsset::ApplyConfigurationToSpawner: Spawner is null"));
		return;
	}

	Spawner->SpawnableActors = Configuration.SpawnableActors;
	Spawner->SpawnTypeConfig = Configuration.SpawnTypeConfig;
	Spawner->SpawnerSettings = Configuration.SpawnerSettings;
	Spawner->AsteroidRandomization = Configuration.AsteroidRandomization;
	Spawner->bAutoStartSpawning = Configuration.bAutoStartSpawning;
	Spawner->bEnabled = Configuration.bEnabled;
	Spawner->GlobalSpawnRateMultiplier = Configuration.GlobalSpawnRateMultiplier;
}