// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/GameStates/AccelByteWarsInGameGameState.h"

#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

#include "CoreMinimal.h"
#include "Core/PowerUps/PowerUpBase.h"
#include "PowerUpWormHole.generated.h"

UCLASS()
class ACCELBYTEWARS_API APowerUpWormHole : public APowerUpBase
{
	GENERATED_BODY()

public:
	APowerUpWormHole();

	/**
	 * @brief Starts the worm hole power up
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_InitiateWormHoleGenerator();

	/**
	 * @brief Informs all players of visual changes
	 */
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable, Category = AccelByteWars)
		void Multicast_InitiateWormHoleGenerator(AActor* ActorToMove, FVector NewPawnLocation);

	/**
	 * @brief Sets the worm hole color
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
		void SetWormHoleColor(FLinearColor InColor);

	/**
	 * @brief Handles end of life for the worm hole
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
		void Server_WormHoleExpired();

	/**
	 * @brief A do-nothing sphere component used for positioning
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		class USphereComponent* SphereComponent = nullptr;
	
	/**
	 * @brief FX for worm hole
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UNiagaraComponent* WormHoleFx = nullptr;

	/**
	 * @brief Sound FX for worm hole
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAudioComponent* WormHoleAudioComponent = nullptr;

	/**
	 * @brief Maximum lifetime of the wormhole
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		float MaxWormHoleLifetime = 5.0f;

	/**
	 * @brief Current lifetime of the wormhole
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
		float CurrentWormHoleLifetime = 0.0f;

	/**
	 * @brief Boolean indicating that the wormhole has been used and is ready to be destroyed
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
		bool MarkAsExpired = false;

	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	//~End of UObject overridden functions

	UFUNCTION(Category = AccelByteWars)
		virtual void DestroyPowerUp() override;
};
