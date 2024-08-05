// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpSplitMissile.h"

#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"

APowerUpSplitMissile::APowerUpSplitMissile()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	bReplicates = true;

	// check if should be triggered or not
	if (const AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner()))
	{
		if (!Pawn->FiredMissile)
		{
			APowerUpSplitMissile::DestroyItem();
			return;
		}

		const AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
		const AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(Pawn->GetPlayerState());
		if (!GameState || !ABPlayerState)
		{
			return;
		}

		if (ABPlayerState->MissilesFired > GameState->GameSetup.FiredMissilesLimit)
		{
			APowerUpSplitMissile::DestroyItem();
			return;
		}
	}
}

void APowerUpSplitMissile::OnUse()
{
	IInGameItemInterface::OnUse();

	if (!HasAuthority())
	{
		return;
	}

	if (AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner()))
	{
		if (!Pawn->FiredMissile)
		{
			DestroyItem();
			return;
		}

		FireMissiles();
		Pawn->FireMissileFx(ParentMissileTransform);
	}
}

void APowerUpSplitMissile::OnEquip()
{
	IInGameItemInterface::OnEquip();
}

void APowerUpSplitMissile::DestroyItem()
{
	Destroy();
}

void APowerUpSplitMissile::FireMissiles()
{
	AAccelByteWarsPlayerPawn* Pawn = Cast<AAccelByteWarsPlayerPawn>(Owner);
	if (!Pawn)
	{
		return;
	}

	// Calculate missile spawn location
	const FTransform SpawnTransform = CalculateWhereToSpawnMissile(Pawn->FiredMissile->GetActorTransform());

	// Clamp initial missile speed
	const float ClampedInitialSpeed = UKismetMathLibrary::MapRangeClamped(
		Pawn->FirePowerLevel,
		0.0f,
		1.0f,
		Pawn->MinMissileSpeed,
		Pawn->MaxMissileSpeed);

	// Spawn left missile actor
	AAccelByteWarsMissile* LeftFiredMissile = SpawnMissile(
		Pawn,
		SpawnTransform,
		ClampedInitialSpeed,
		Pawn->GetPawnColor(),
		Pawn->MissileActor);
	if (LeftFiredMissile == nullptr)
		return;

	// Rotate left missile 90 degrees
	LeftFiredMissile->Velocity = LeftFiredMissile->InitialSpeed * -GetActorTransform().GetRotation().GetForwardVector();

	// Spawn right missile actor
	AAccelByteWarsMissile* RightFiredMissile = SpawnMissile(Pawn, SpawnTransform, ClampedInitialSpeed, Pawn->GetPawnColor(), Pawn->MissileActor);
	if (RightFiredMissile == nullptr)
		return;

	// Rotate right missile 90 degrees
	RightFiredMissile->Velocity = RightFiredMissile->InitialSpeed * GetActorTransform().GetRotation().GetForwardVector();

	// Spawn left missile trail
	AAccelByteWarsMissileTrail* LeftMissileTrail = SpawnBPActorInWorld<AAccelByteWarsMissileTrail>(
		Pawn,
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		Pawn->MissileTrailActor,
		true);
	if (LeftMissileTrail == nullptr)
	{
		return;
	}

	// Spawn right missile trail
	AAccelByteWarsMissileTrail* RightMissileTrail = SpawnBPActorInWorld<AAccelByteWarsMissileTrail>(
		Pawn,
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		Pawn->MissileTrailActor,
		true);
	if (LeftMissileTrail == nullptr)
	{
		return;
	}

	// Set Trail colors
	LeftMissileTrail->Server_SetColor(Pawn->GetPawnColor());
	RightMissileTrail->Server_SetColor(Pawn->GetPawnColor());

	// Attach left/right trail to left/right missile actor
	const FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepRelative,
		true);
	LeftMissileTrail->AttachToActor(LeftFiredMissile, AttachmentRules);
	RightMissileTrail->AttachToActor(RightFiredMissile, AttachmentRules);

	// Increment missiles fired
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(Pawn->GetPlayerState());
	if (ABPlayerState == nullptr)
		return;

	ABPlayerState->MissilesFired += 2;

	DestroyItem();
}

AAccelByteWarsMissile* APowerUpSplitMissile::SpawnMissile(
	APawn* SplitMissileOwner,
	FTransform SpawnTransform,
	float InClampedInitialSpeed,
	FLinearColor InColor,
	TSubclassOf<AActor> InFiredMissileActor)
{
	// Spawn right missile actor
	AAccelByteWarsMissile* FiredMissile = SpawnBPActorInWorld<AAccelByteWarsMissile>(SplitMissileOwner, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), InFiredMissileActor, true);
	if (FiredMissile == nullptr)
		return nullptr;

	FiredMissile->Server_SetColor(InColor);
	FiredMissile->InitialSpeed = InClampedInitialSpeed;

	return FiredMissile;
}

template<class T>
T* APowerUpSplitMissile::SpawnBPActorInWorld(
	APawn* OwningPawn,
	const FVector Location,
	const FRotator Rotation,
	TSubclassOf<AActor> ActorClass,
	bool ShouldReplicate)
{
	if (OwningPawn == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwningPawn;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	T* NewActor = OwningPawn->GetWorld()->SpawnActor<T>(ActorClass, FTransform(Rotation, Location), SpawnParameters);
	if (NewActor == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate actor class for: " + ActorClass->GetName());
		return nullptr;
	}

	NewActor->SetInstigator(OwningPawn);
	NewActor->SetReplicates(ShouldReplicate);

	return NewActor;
}

FTransform APowerUpSplitMissile::CalculateWhereToSpawnMissile(const FTransform InParentTransform)
{
	FTransform OutTransform;
	OutTransform.SetLocation(InParentTransform.GetLocation());
	OutTransform.SetRotation(InParentTransform.GetRotation());
	OutTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	return OutTransform;
}