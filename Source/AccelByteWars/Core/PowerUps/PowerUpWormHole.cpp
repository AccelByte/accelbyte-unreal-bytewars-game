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

	// Set color
	if (const AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner()))
	{
		Color = Pawn->GetPawnColor();
		OnRepNotify_Color();
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

void APowerUpWormHole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpWormHole, Color);
}

void APowerUpWormHole::Server_InitiateWormHoleGenerator_Implementation()
{
	if (!GetOwner()) 
	{
		return;
	}

	AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner());
	if (!Pawn) 
	{
		return;
	}

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

	// Find new location
	FVector2D Position2D;
	if (!InGameGameMode->FindGoodSpawnLocation(Position2D))
	{
		return;
	}

	const FVector NewPawnLocation = {Position2D.X, Position2D.Y, 0.0f};;
	Multicast_InitiateWormHoleGenerator(Pawn, NewPawnLocation);
	Pawn->SetActorLocation(NewPawnLocation);

	MarkAsExpired = true;
}

void APowerUpWormHole::Multicast_InitiateWormHoleGenerator_Implementation(AActor* ActorToMove, FVector NewPawnLocation)
{
	if (WormHoleFx) 
	{
		WormHoleFx->Activate(true);
	}

	ActorToMove->SetActorLocation(NewPawnLocation);
}

void APowerUpWormHole::WormHoleExpired()
{
	if (WormHoleFx) 
	{
		WormHoleFx->Deactivate();
	}

	if (WormHoleAudioComponent) 
	{
		WormHoleAudioComponent->Stop();
	}

	Destroy();
}

void APowerUpWormHole::OnRepNotify_Color()
{
	if (WormHoleFx)
	{
		WormHoleFx->SetVariableLinearColor(FName(NiagaraVariableColorName), Color);
	}
}