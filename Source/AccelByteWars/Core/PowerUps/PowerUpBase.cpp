// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpBase.h"

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

// Sets default values
APowerUpBase::APowerUpBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APowerUpBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APowerUpBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}