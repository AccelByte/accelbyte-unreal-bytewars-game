// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"

#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "PowerUpSplitMissile.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API APowerUpSplitMissile : public AActor, public IInGameItemInterface
{
	GENERATED_BODY()

	virtual void OnUse() override;
	virtual void OnEquip() override;
	virtual void DestroyItem() override;
	virtual EItemType GetType() override { return EItemType::PowerUp; }

public:
	APowerUpSplitMissile();

	/**
	 * @brief Shoots a missile
	 */
	UFUNCTION(BlueprintCallable,Category = AccelByteWars)
	void FireMissiles();

	/**
	 * @brief Calculates where to spawn left missile
	 */
	UFUNCTION(Category = AccelByteWars)
	AAccelByteWarsMissile* SpawnMissile(
		APawn* SplitMissileOwner,
		FTransform SpawnTransform,
		float InClampedInitialSpeed,
		FLinearColor InColor,
		TSubclassOf<AActor> InFiredMissileActor);

	template<class T>
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	T* SpawnBPActorInWorld(
		APawn* OwningPawn,
		const FVector Location,
		const FRotator Rotation,
		TSubclassOf<AActor> ActorClass,
		bool ShouldReplicate);

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
