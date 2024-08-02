// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Components/AccelByteWarsProceduralMeshComponent.h"
#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "GameFramework/Actor.h"
#include "PlayerShipBase.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API APlayerShipBase : public AActor, public IInGameItemInterface
{
	GENERATED_BODY()

	virtual void OnEquip() override;
	virtual void OnUse() override;

public:
	// Sets default values for this actor's properties
	APlayerShipBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRepNotifyGlowModifier)
	float GlowModifier = 1.0f;

	UFUNCTION()
	void OnRepNotifyGlowModifier();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Visible mesh of player ship
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsProceduralMeshComponent* AccelByteWarsProceduralMesh = nullptr;

	virtual void SetShipColor(FLinearColor InColor) {}

	/** @brief Set and update glow modifier to all connected client */
	void SetGlowModifier(const float Modifier);
};
