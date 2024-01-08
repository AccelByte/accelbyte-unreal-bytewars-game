// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Player/AccelByteWarsPlayerState.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissileTrail.h"

#include "Kismet/KismetMathLibrary.h"

#include "CoreMinimal.h"
#include "Core/PowerUps/PowerUpBase.h"
#include "PowerUpSplitMissile.generated.h"

UCLASS()
class ACCELBYTEWARS_API APowerUpSplitMissile : public APowerUpBase
{
	GENERATED_BODY()

public:
	APowerUpSplitMissile();

	/**
	 * @brief Shoots a missile
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_FireMissiles(APawn* SplitMissileOwner, FTransform InParentTransform, float InFirePowerLevel, float InMinMissileSpeed, float InMaxMissileSpeed, FLinearColor InColor, const FString& InFiredMissileBlueprintPath, const FString& InFiredMissileTrailBlueprintPath);

	UFUNCTION(Category = AccelByteWars)
		virtual void DestroyPowerUp() override;

	/**
	 * @brief Calculates where to spawn left missile
	 */
	UFUNCTION(Category = AccelByteWars)
		AAccelByteWarsMissile* SpawnMissile(APawn* SplitMissileOwner, FTransform SpawnTransform, float InClampedInitialSpeed, FLinearColor InColor, FString InFiredMissileBlueprintPath);

	template<class T>
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		T* SpawnBPActorInWorld(APawn* OwningPawn, const FVector Location, const FRotator Rotation, FString BlueprintPath, bool ShouldReplicate);

	/**
	 * @brief Calculates where to spawn missile
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		FTransform CalculateWhereToSpawnMissile(FTransform InParentTransform);

	/**
	 * @brief Transform of the parent missile to split from
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
		FTransform ParentMissileTransform;
	
};
