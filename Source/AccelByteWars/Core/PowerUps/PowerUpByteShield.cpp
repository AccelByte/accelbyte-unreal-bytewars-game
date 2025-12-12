// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpByteShield.h"

#include "Core/Actor/AccelByteWarsMissile.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Kismet/GameplayStatics.h"

#include "Kismet/KismetMathLibrary.h"

APowerUpByteShield::APowerUpByteShield()
{
	// check if owning pawn already has a shield up
	if (const AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(Owner))
	{
		if (Pawn->GetActivePowerUp())
		{
			APowerUpByteShield::DestroyItem();
		}
	}

	// Setup a do nothing root component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldSphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;

	// Setup shield fx
	ShieldFx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShieldNiagaraComponent"));
	ShieldFx->SetWorldScale3D(FVector(2.5f, 2.5f, 2.5f));
	ShieldFx->SetupAttachment(RootComponent);

	// Setup audio component
	ShieldAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShieldAudioComponent"));
	ShieldAudioComponent->SetupAttachment(RootComponent);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AActor::SetReplicateMovement(true);
	bReplicates = true;
}

void APowerUpByteShield::Tick(float DeltaTime)
{
	if (IsActive && HasAuthority())
	{
		CurrentCollisionTickRate += DeltaTime;
		CurrentShieldLifetime += DeltaTime;

		if (CurrentShieldLifetime > MaxShieldLifetime)
		{
			ShieldExpired();
			IsActive = false;
		}

		CheckCollision();
		CurrentCollisionTickRate = 0.0f;
	}
}

void APowerUpByteShield::OnUse()
{
	IInGameItemInterface::OnUse();

	if (!HasAuthority())
	{
		return;
	}

	if (AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner()))
	{
		Color = Pawn->GetPawnColor();
		AttachToActor(Pawn, FAttachmentTransformRules::SnapToTargetIncludingScale);
		OnRepNotify_Color();
	}
}

void APowerUpByteShield::OnEquip()
{
	IInGameItemInterface::OnEquip();
}

void APowerUpByteShield::DestroyItem()
{
	ShieldExpired();
}

void APowerUpByteShield::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpByteShield, Color);
}

void APowerUpByteShield::CheckCollision()
{
	if (!Owner)
	{
		return;
	}

	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);

	for (int32 Index = 0; Index < FoundMissiles.Num(); Index++)
	{
		AAccelByteWarsMissile* Missile = Cast<AAccelByteWarsMissile>(FoundMissiles[Index]);
		if (!Missile) 
		{
			return;
		}

		float Distance = UKismetMathLibrary::Vector_Distance(this->GetActorLocation(), Missile->GetActorLocation());
		if (Distance > ShieldRadius) 
		{
			continue;
		}

		if (HasAuthority()) 
		{
			ShieldHitByMissile(Missile);
		}
	}
}

void APowerUpByteShield::ShieldHitByMissile(AAccelByteWarsMissile* Missile)
{
	// Destroy the missile and the shield
	Missile->DestroyByPowerUp();
	ShieldExpired();

	IsActive = false;
}

void APowerUpByteShield::ShieldExpired()
{
	if (ShieldFx) 
	{
		ShieldFx->Deactivate();
	}

	if (ShieldAudioComponent) 
	{
		ShieldAudioComponent->Stop();
	}

	Destroy();
}

void APowerUpByteShield::OnRepNotify_Color()
{
	if (ShieldFx) 
	{
		ShieldFx->SetVariableLinearColor(FName(NiagaraVariableColorName), Color);
	}
}