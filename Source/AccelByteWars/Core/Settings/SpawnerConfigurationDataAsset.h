// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/Actor/AccelByteWarsSpawner.h"
#include "SpawnerConfigurationDataAsset.generated.h"

USTRUCT(BlueprintType)
struct ACCELBYTEWARS_API FSpawnerConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	TArray<FSpawnableActorData> SpawnableActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	TMap<ESpawnableActorType, FSpawnableConfig> SpawnTypeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	FSpawnerSettings SpawnerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	FAsteroidRandomizationSettings AsteroidRandomization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	bool bAutoStartSpawning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration")
	float GlobalSpawnRateMultiplier = 1.0f;

	FSpawnerConfiguration()
	{
		SpawnableActors.Empty();
		SpawnerSettings = FSpawnerSettings();
		AsteroidRandomization = FAsteroidRandomizationSettings();
		bAutoStartSpawning = true;
		bEnabled = true;
		GlobalSpawnRateMultiplier = 1.0f;
	}
};

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API USpawnerConfigurationDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	USpawnerConfigurationDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return CodeName.IsEmpty() ? Super::GetPrimaryAssetId() : FPrimaryAssetId(USpawnerConfigurationDataAsset::StaticClass()->GetFName(), FName(*CodeName));
	}

	static USpawnerConfigurationDataAsset* GetByCodeName(const FString& InCodeName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Configuration", meta = (ShowOnlyInnerProperties))
	FSpawnerConfiguration Configuration;

	UFUNCTION(BlueprintCallable, Category = "Spawner Configuration")
	void ApplyConfigurationToSpawner(AAccelByteWarsSpawner* Spawner) const;
};