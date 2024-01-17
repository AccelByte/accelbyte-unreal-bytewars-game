// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpSplitMissile.h"

APowerUpSplitMissile::APowerUpSplitMissile()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	bReplicates = true;
}

void APowerUpSplitMissile::Server_FireMissiles_Implementation(APawn* SplitMissileOwner, FTransform InParentTransform, float InFirePowerLevel, float InMinMissileSpeed, float InMaxMissileSpeed, FLinearColor InColor, const FString& InFiredMissileBlueprintPath, const FString& InFiredMissileTrailBlueprintPath)
{
	if (SplitMissileOwner == nullptr)
		return;

	// Calculate missile spawn location
	FTransform SpawnTransform = CalculateWhereToSpawnMissile(InParentTransform);

	// Clamp initial missile speed
	float clamped_initial_speed = UKismetMathLibrary::MapRangeClamped(InFirePowerLevel, 0.0f, 1.0f, InMinMissileSpeed, InMaxMissileSpeed);

	// Spawn left missile actor
	AAccelByteWarsMissile* LeftFiredMissile = SpawnMissile(SplitMissileOwner, SpawnTransform, clamped_initial_speed, InColor, InFiredMissileBlueprintPath);
	if (LeftFiredMissile == nullptr)
		return;

	// Rotate left missile 90 degrees
	LeftFiredMissile->Velocity = LeftFiredMissile->InitialSpeed * -GetActorTransform().GetRotation().GetForwardVector();

	// Spawn right missile actor
	AAccelByteWarsMissile* RightFiredMissile = SpawnMissile(SplitMissileOwner, SpawnTransform, clamped_initial_speed, InColor, InFiredMissileBlueprintPath);
	if (RightFiredMissile == nullptr)
		return;

	// Rotate right missile 90 degrees
	RightFiredMissile->Velocity = RightFiredMissile->InitialSpeed * GetActorTransform().GetRotation().GetForwardVector();

	// Spawn left missile trail
	AAccelByteWarsMissileTrail* LeftMissileTrail = SpawnBPActorInWorld<AAccelByteWarsMissileTrail>(SplitMissileOwner, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), InFiredMissileTrailBlueprintPath, true);
	if (LeftMissileTrail == nullptr)
		return;

	// Spawn right missile trail
	AAccelByteWarsMissileTrail* RightMissileTrail = SpawnBPActorInWorld<AAccelByteWarsMissileTrail>(SplitMissileOwner, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), InFiredMissileTrailBlueprintPath, true);
	if (LeftMissileTrail == nullptr)
		return;

	// Set Trail colors
	LeftMissileTrail->Server_SetColor(InColor);
	RightMissileTrail->Server_SetColor(InColor);

	// Attach left/right trail to left/right missile actor
	FAttachmentTransformRules attachment_rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	LeftMissileTrail->AttachToActor(LeftFiredMissile, attachment_rules);
	RightMissileTrail->AttachToActor(RightFiredMissile, attachment_rules);

	// Increment missiles fired
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(SplitMissileOwner->GetPlayerState());
	if (ABPlayerState == nullptr)
		return;

	ABPlayerState->MissilesFired += 2;
}

AAccelByteWarsMissile* APowerUpSplitMissile::SpawnMissile(APawn* SplitMissileOwner, FTransform SpawnTransform, float InClampedInitialSpeed, FLinearColor InColor, FString InFiredMissileBlueprintPath)
{
	// Spawn right missile actor
	AAccelByteWarsMissile* FiredMissile = SpawnBPActorInWorld<AAccelByteWarsMissile>(SplitMissileOwner, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), InFiredMissileBlueprintPath, true);
	if (FiredMissile == nullptr)
		return nullptr;

	FiredMissile->Server_SetColor(InColor);
	FiredMissile->InitialSpeed = InClampedInitialSpeed;

	return FiredMissile;
}

template<class T>
T* APowerUpSplitMissile::SpawnBPActorInWorld(APawn* OwningPawn, const FVector Location, const FRotator Rotation, FString BlueprintPath, bool ShouldReplicate)
{
	if (OwningPawn == nullptr)
		return nullptr;

	FActorSpawnParameters spawn_parameters;
	spawn_parameters.Owner = OwningPawn;
	spawn_parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* generic_class = StaticLoadClass(AActor::StaticClass(), OwningPawn, *BlueprintPath);
	if (generic_class == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate generic class for: " + BlueprintPath);
		return nullptr;
	}

	T* new_actor = OwningPawn->GetWorld()->SpawnActor<T>(generic_class, FTransform(Rotation, Location), spawn_parameters);
	if (new_actor == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate actor class for: " + BlueprintPath);
		return nullptr;
	}

	new_actor->SetInstigator(OwningPawn);
	new_actor->SetReplicates(ShouldReplicate);

	return new_actor;
}

FTransform APowerUpSplitMissile::CalculateWhereToSpawnMissile(FTransform InParentTransform)
{
	FTransform out_transform;
	out_transform.SetLocation(InParentTransform.GetLocation());
	out_transform.SetRotation(InParentTransform.GetRotation());
	out_transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	return out_transform;
}

void APowerUpSplitMissile::DestroyPowerUp()
{
	Destroy();
}