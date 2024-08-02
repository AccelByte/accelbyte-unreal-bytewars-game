// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "PowerUpByteBomb.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API APowerUpByteBomb : public AActor, public IInGameItemInterface
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void OnUse() override;
	virtual void OnEquip() override;
	virtual EItemType GetType() override { return EItemType::PowerUp; }

	UFUNCTION()
	virtual void DestroyItem() override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> ShakeClass;

	APowerUpByteBomb();

	/**
	 * @brief Simple EZ to use camera shake, called from BP
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void ShakeCamera();

	/**
	 * @brief Destroys all enemy missiles
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void DestroyAllEnemyMissiles(int32 TeamId);
	
	/**
	* @brief A do-nothing sphere component used for positioning
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	class USphereComponent* SphereComponent = nullptr;

	/**
	* @brief Sound FX for bomb
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UAudioComponent* BombAudioComponent = nullptr;

protected:
	int32 GetTeamIdFromPawn(const AActor* Actor) const;
};
