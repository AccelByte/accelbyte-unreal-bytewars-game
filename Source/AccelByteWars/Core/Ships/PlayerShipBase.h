// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Components/AccelByteWarsProceduralMeshComponent.h"
#include "PlayerShipModels.h"
#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerShipBase.generated.h"

UCLASS()
class ACCELBYTEWARS_API APlayerShipBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerShipBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Visible mesh of player ship
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsProceduralMeshComponent* AccelByteWarsProceduralMesh = nullptr;

	virtual void SetShipColor(FLinearColor InColor) {}

};
