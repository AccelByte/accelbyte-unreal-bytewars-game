// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpWormHole.h"

#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"

APowerUpWormHole::APowerUpWormHole()
{
	// Setup a do nothing root component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldSphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;

	// Setup shield fx
	WormHoleFx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShieldNiagaraComponent"));
	WormHoleFx->SetWorldScale3D(FVector(2.5f, 2.5f, 2.5f));
	WormHoleFx->SetupAttachment(RootComponent);
	WormHoleFx->bAutoActivate = false;

	// Setup audio component
	WormHoleAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShieldAudioComponent"));
	WormHoleAudioComponent->SetupAttachment(RootComponent);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	bReplicates = true;
}

void APowerUpWormHole::Tick(float DeltaTime)
{
	if (MarkAsExpired && HasAuthority())
	{	
		CurrentWormHoleLifetime += DeltaTime;
		if (CurrentWormHoleLifetime > MaxWormHoleLifetime)
		{
			MarkAsExpired = false;
			WormHoleExpired();
		}
	}
}

void APowerUpWormHole::OnUse()
{
	IInGameItemInterface::OnUse();

	if (!HasAuthority())
	{
		return;
	}

	// set color
	if (const AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner()))
	{
		SetWormHoleColor(Pawn->GetPawnColor());
	}
	Server_InitiateWormHoleGenerator();
}

void APowerUpWormHole::OnEquip()
{
	IInGameItemInterface::OnEquip();
}

void APowerUpWormHole::DestroyItem()
{
	WormHoleExpired();
}

void APowerUpWormHole::Server_InitiateWormHoleGenerator_Implementation()
{
	if (GetOwner() == nullptr)
		return;

	AAccelByteWarsPlayerPawn* ABPawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner());
	if (ABPawn == nullptr)
		return;

	// find new location
	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (!GameModeBase)
	{
		return;
	}
	AAccelByteWarsInGameGameMode* InGameGameMode = Cast<AAccelByteWarsInGameGameMode>(GameModeBase);
	if (!InGameGameMode)
	{
		return;
	}
	FVector2D Position2D;
	if (!InGameGameMode->FindGoodSpawnLocation(Position2D))
	{
		return;
	}
	const FVector NewPawnLocation = {Position2D.X, Position2D.Y, 0.0f};;

	Multicast_InitiateWormHoleGenerator(ABPawn, NewPawnLocation);

	ABPawn->SetActorLocation(NewPawnLocation);

	MarkAsExpired = true;
}

void APowerUpWormHole::Multicast_InitiateWormHoleGenerator_Implementation(AActor* ActorToMove, FVector NewPawnLocation)
{
	if (WormHoleFx != nullptr)
		WormHoleFx->Activate(true);

	ActorToMove->SetActorLocation(NewPawnLocation);
}

void APowerUpWormHole::WormHoleExpired()
{
	if (WormHoleFx != nullptr)
		WormHoleFx->Deactivate();

	if (WormHoleAudioComponent != nullptr)
		WormHoleAudioComponent->Stop();

	Destroy();
}