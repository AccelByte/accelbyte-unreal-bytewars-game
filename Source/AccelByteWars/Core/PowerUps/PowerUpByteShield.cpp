// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpByteShield.h"

#include "Core/Actor/AccelByteWarsMissile.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"

#include "Kismet/KismetMathLibrary.h"

APowerUpByteShield::APowerUpByteShield()
{
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
	SetReplicateMovement(true);
	bReplicates = true;
}

// Called every frame
void APowerUpByteShield::Tick(float DeltaTime)
{
	if (IsShieldActive)
	{
		CurrentCollisionTickRate += DeltaTime;
		CurrentShieldLifetime += DeltaTime;

		if (CurrentShieldLifetime > MaxShieldLifetime)
		{
			Server_ShieldExpired();
			IsShieldActive = false;
		}

		CheckCollision();
		CurrentCollisionTickRate = 0.0f;
	}
}

void APowerUpByteShield::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpByteShield, IsShieldActive);
}

void APowerUpByteShield::CheckCollision()
{
	if (Owner == nullptr)
		return;

	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);

	for (int i = 0; i < FoundMissiles.Num(); i++)
	{
		AAccelByteWarsMissile* ABMissile = Cast<AAccelByteWarsMissile>(FoundMissiles[i]);
		if (ABMissile == nullptr)
			return;

		float Distance = UKismetMathLibrary::Vector_Distance(this->GetActorLocation(), ABMissile->GetActorLocation());
		if (Distance > ShieldRadius)
			continue;

		if (HasAuthority())
			Server_ShieldHitByMissile(ABMissile);
	}
}

void APowerUpByteShield::Server_ShieldHitByMissile_Implementation(AAccelByteWarsMissile* ABMissile)
{
	// Destroy the missile and the shield
	ABMissile->DestroyByPowerUp();
	Server_ShieldExpired();

	IsShieldActive = false;
	OnRepNotify_IsShieldActive();
}

void APowerUpByteShield::Server_ShieldExpired_Implementation()
{
	if (ShieldFx != nullptr)
		ShieldFx->Deactivate();

	if (ShieldAudioComponent != nullptr)
		ShieldAudioComponent->Stop();

	Destroy();
}

void APowerUpByteShield::DestroyPowerUp()
{
	Server_ShieldExpired();
}