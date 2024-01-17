// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

#include "NiagaraComponent.h"

#include "CoreMinimal.h"
#include "Core/PowerUps/PowerUpBase.h"
#include "PowerUpByteShield.generated.h"

UCLASS()
class ACCELBYTEWARS_API APowerUpByteShield : public APowerUpBase
{
	GENERATED_BODY()

public:

	APowerUpByteShield();

	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	//~End of UObject overridden functions

	/**
	 * @brief When another actor enters the shield
	 */
	UFUNCTION(Category = AccelByteWars)
		void CheckCollision();

	/**
	 * @brief Kills the shield and the missile hitting the shield
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_ShieldHitByMissile(AAccelByteWarsMissile* ABMissile);

	/**
	 * @brief Sets the shield color
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
		void SetShieldColor(FLinearColor InColor);

	/**
	 * @brief A do-nothing sphere component used for positioning
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		class USphereComponent* SphereComponent = nullptr;
	
	/**
	 * @brief FX for shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UNiagaraComponent* ShieldFx = nullptr;

	/**
	 * @brief Sound FX for shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAudioComponent* ShieldAudioComponent = nullptr;

	/**
	 * @brief Maximum lifetime of the shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_IsShieldActive)
		float IsShieldActive = true;

	/**
	 * @brief Collision detection radius for the shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		float ShieldRadius = 95.0f;

	/**
	 * @brief Maximum lifetime of the shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		float MaxShieldLifetime = 5.0f;

	/**
	 * @brief Current lifetime of the shield
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
		float CurrentShieldLifetime = 0.0f;

	/**
	 * @brief Maximum lifetime of the shield
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		float MaxCollisionTickRate = 0.0f;

	/**
	 * @brief Current lifetime of the shield
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
		float CurrentCollisionTickRate = 0.0f;

	/**
	 * @brief Handles end of life for the shield
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_ShieldExpired();

	/**
	 * @brief Generic OnRep notify for detecting if the shield is active
	 */
	UFUNCTION()
		void OnRepNotify_IsShieldActive() { /* Do Nothing */ };

	UFUNCTION(Category = AccelByteWars)
		virtual void DestroyPowerUp() override;
};
