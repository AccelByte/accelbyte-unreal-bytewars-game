// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/PowerUps/PowerUpBase.h"
#include "PowerUpByteBomb.generated.h"

UCLASS()
class ACCELBYTEWARS_API APowerUpByteBomb : public APowerUpBase
{
	GENERATED_BODY()

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	//~End of UObject overridden functions

public:

	APowerUpByteBomb();
	
	/**
	 * @brief Explodes the Byte Bomb
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
		void InitiateExplosion();

	/**
	 * @brief Simple EZ to use camera shake, called from BP
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_ShakeCamera(APawn* ByteBombOwner, TSubclassOf<class UCameraShakeBase> Shake);

	/**
	 * @brief Destroys all enemy missiles
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_DestroyAllEnemyMissiles(APawn* ByteBombOwner, int32 TeamId);

	UFUNCTION(Category = AccelByteWars)
		virtual void DestroyPowerUp() override;
	
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
};
