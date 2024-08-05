// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "PowerUpWormHole.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API APowerUpWormHole : public AActor, public IInGameItemInterface
{
	GENERATED_BODY()
	
	virtual void Tick(float DeltaTime) override;
	virtual void OnUse() override;
	virtual void OnEquip() override;
	virtual void DestroyItem() override;
	virtual EItemType GetType() override { return EItemType::PowerUp; }

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
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void WormHoleExpired();

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
};
