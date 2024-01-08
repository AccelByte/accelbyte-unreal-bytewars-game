// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Components/AccelByteWarsProceduralMeshComponent.h"

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpBase.generated.h"

UCLASS()
class ACCELBYTEWARS_API APowerUpBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APowerUpBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Destroys power up after use
	virtual void DestroyPowerUp() {};

};
