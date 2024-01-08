// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpByteBomb.h"

#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerPawn.h"

APowerUpByteBomb::APowerUpByteBomb()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(false);
	bReplicates = true;
}

void APowerUpByteBomb::BeginPlay()
{
	Super::BeginPlay();
}

void APowerUpByteBomb::Server_ShakeCamera_Implementation(APawn* ByteBombOwner, TSubclassOf<class UCameraShakeBase> Shake)
{
	if (ByteBombOwner == nullptr)
		return;

	for (FConstPlayerControllerIterator Iterator = ByteBombOwner->GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
			return;

		PlayerController->ClientStartCameraShake(Shake, 0.2f);
	}
}

void APowerUpByteBomb::Server_DestroyAllEnemyMissiles_Implementation(APawn* ByteBombOwner, int32 TeamId)
{
	if (ByteBombOwner == nullptr)
		return;

	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(ByteBombOwner->GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);

	for (int i = 0; i < FoundMissiles.Num(); i++)
	{
		AAccelByteWarsMissile* Missile = Cast<AAccelByteWarsMissile>(FoundMissiles[i]);
		if (Missile == nullptr)
			continue;

		AAccelByteWarsPlayerPawn* MissileOwner = Cast<AAccelByteWarsPlayerPawn>(Missile->Owner);
		if (MissileOwner == nullptr)
			continue;

		AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(MissileOwner->GetController());
		if (ABPlayerController == nullptr)
			continue;

		if (ABPlayerController->PlayerState == nullptr)
			continue;

		AAccelByteWarsPlayerState* ABPlayerState = static_cast<AAccelByteWarsPlayerState*>(ABPlayerController->PlayerState);
		if (ABPlayerState == nullptr)
			continue;

		if (ABPlayerState->TeamId != TeamId)
		{
			Missile->DestroyByPowerUp();
		}
	}

	DestroyPowerUp();
}

void APowerUpByteBomb::DestroyPowerUp()
{
	Destroy();
}