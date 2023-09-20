// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Player/AccelByteWarsPlayerPawn.h"

// Sets default values
AAccelByteWarsPlayerPawn::AAccelByteWarsPlayerPawn()
{
	// Setup procedural mesh component
	AccelByteWarsProceduralMesh = CreateDefaultSubobject<UAccelByteWarsProceduralMeshComponent>(TEXT("AccelByteWarsProceduralMesh"));
	RootComponent = AccelByteWarsProceduralMesh;

	// Setup ship gameplay object
	GameplayObject = CreateDefaultSubobject<UAccelByteWarsGameplayObjectComponent>(TEXT("GameplayObject"));
	GameplayObject->Mass = 1.0f;
	GameplayObject->Radius = 0.5f;
	GameplayObject->ObjectType = EGameplayObjectType::SHIP;

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AAccelByteWarsPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentYaw = GetActorRotation().Yaw;
}

// Called every frame
void AAccelByteWarsPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->IsNetMode(NM_DedicatedServer) == false)
	{
		UpdatePowerBarUI(DeltaTime);
	}

	if (HasAuthority())
	{
		// Destroy and remove index
		if (MissileTrail != nullptr)
		{
			if (MissileTrail->IsFadeOut())
				MissileTrail->Destroy();
		}

		// Adjust fire power
		if (FirePowerAdjustRate != 0.0f)
		{
			float a = DeltaTime * FirePowerAdjustRate * ConstFirePowerAdjustRate;
			float b = FirePowerLevel + a;

			FirePowerLevel = FMath::Clamp(b, 0.0f, 1.0f);
		}

		if (RotationDirection != 0)
		{
			if (RotationDirection == 1) // Clock-wise
			{
				CurrentYaw += (ConstRotateRate * -1.0f);
			}
			else if (RotationDirection == 2) // Counter clock-wise
			{
				CurrentYaw += ConstRotateRate;
			}

			OnRepNotify_CurrentYaw();
		}
	}
}

void AAccelByteWarsPlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerPawn, RotationDirection);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, CurrentYaw);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, FirePowerLevel);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, Color);
}

AAccelByteWarsMissile* AAccelByteWarsPlayerPawn::SpawnMissileInWorld(AActor* ActorOwner, FTransform InTransform, float InitialSpeed, FString BlueprintPath, bool ShouldReplicate)
{
	if (ActorOwner == nullptr)
		return nullptr;

	FActorSpawnParameters spawn_parameters;
	spawn_parameters.Owner = ActorOwner;
	spawn_parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* generic_class = StaticLoadClass(AActor::StaticClass(), ActorOwner, *BlueprintPath);

	AAccelByteWarsMissile* new_actor = ActorOwner->GetWorld()->SpawnActor<AAccelByteWarsMissile>(generic_class, InTransform, spawn_parameters);
	if (new_actor == nullptr)
		return nullptr;

	new_actor->Server_SetColor(Color);

	new_actor->InitialSpeed = InitialSpeed;
	new_actor->SetVelocity();
	new_actor->SetInstigator(this);
	new_actor->SetReplicates(ShouldReplicate);

	return new_actor;
}

AAccelByteWarsMissileTrail* AAccelByteWarsPlayerPawn::SpawnMissileTrailInWorld(AActor* ActorOwner, FTransform InTransform, FString BlueprintPath, bool ShouldReplicate)
{
	if (ActorOwner == nullptr)
		return nullptr;

	FActorSpawnParameters spawn_parameters;
	spawn_parameters.Owner = ActorOwner;
	spawn_parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* generic_class = StaticLoadClass(AActor::StaticClass(), ActorOwner, *BlueprintPath);

	AAccelByteWarsMissileTrail* new_actor = ActorOwner->GetWorld()->SpawnActor<AAccelByteWarsMissileTrail>(generic_class, InTransform, spawn_parameters);
	if (new_actor == nullptr)
		return nullptr;

	new_actor->Server_SetColor(Color);
	new_actor->SetInstigator(this);
	new_actor->SetReplicates(ShouldReplicate);

	return new_actor;
}

void AAccelByteWarsPlayerPawn::Server_FireMissile_Implementation()
{
	if (ShouldFire() == false)
		return;

	if (AccelByteWarsProceduralMesh == nullptr)
		return;

	// Calculate missile spawn location
	FTransform SpawnTransform = CalculateWhereToSpawnMissile();

	// Clamp initial missile speed
	float clamped_initial_speed = UKismetMathLibrary::MapRangeClamped(FirePowerLevel, 0.0f, 1.0f, MinMissileSpeed, MaxMissileSpeed);

	// Spawn missile actor
	FiredMissile = SpawnMissileInWorld(this, SpawnTransform, clamped_initial_speed, FiredMissileBlueprintPath, true);
	if (FiredMissile == nullptr)
		return;

	ShowPowerLevelUITimer = 0.0f;

	// Increment missiles fired
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(GetPlayerState());
	if (ABPlayerState == nullptr)
		return;

	ABPlayerState->MissilesFired++;

	// Play sound for missile fire
	FireMissileFx(SpawnTransform);

	if (MissileTrail != nullptr)
	{
		MissileTrail->TriggerFadeOut();
		MissileTrail = nullptr;
	}

	// Spawn missile trail
	MissileTrail = SpawnMissileTrailInWorld(this, SpawnTransform, FiredMissileTrailBlueprintPath, true);
	if (MissileTrail == nullptr)
		return;

	// Attach new trail to missile actor
	FAttachmentTransformRules attachment_rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	MissileTrail->AttachToActor(FiredMissile, attachment_rules);
}

FTransform AAccelByteWarsPlayerPawn::CalculateWhereToSpawnMissile()
{
	FTransform actor_transform = GetActorTransform();
	FVector right_vector = actor_transform.GetRotation().GetRightVector();
	FVector a = (right_vector * 90.0f);
	FVector b = a + actor_transform.GetLocation();

	FTransform out_transform;
	out_transform.SetLocation(b);
	out_transform.SetRotation(actor_transform.GetRotation());
	out_transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	return out_transform;
}

bool AAccelByteWarsPlayerPawn::ShouldFire()
{
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(GetPlayerState());
	if (ABPlayerState == nullptr)
		return false;

	AAccelByteWarsInGameGameState* const ABInGameState = GetWorld()->GetGameState<AAccelByteWarsInGameGameState>();
	if (ABInGameState == nullptr)
		return false;

	if (ABPlayerState->MissilesFired < ABInGameState->GameSetup.FiredMissilesLimit)
		return true;

	return false;
}

void AAccelByteWarsPlayerPawn::AdjustFirePower(int Rate)
{
	if (HasAuthority() == false)
		return;

	FirePowerLevel += FMath::Clamp(Rate, -1, 1);;
}

void AAccelByteWarsPlayerPawn::OnPlayerInputThisFrame()
{
	if (IsDestroyed == false)
		ShowPowerLevelUITimer = 1.0f;
}

void AAccelByteWarsPlayerPawn::Server_RotatePawn_Implementation(int Rate)
{
	Client_RotatePawn_Implementation(Rate);
}

void AAccelByteWarsPlayerPawn::Client_RotatePawn_Implementation(int Rate)
{
	if (HasAuthority())
	{
		int in_rate = FMath::Clamp(Rate, 0, 2);

		RotationDirection = in_rate;
		OnRepNotify_RotationDirection();
	}
}

void AAccelByteWarsPlayerPawn::Server_AdjustFirePower_Implementation(int Rate)
{
	if (HasAuthority())
	{
		int in_rate = FMath::Clamp(Rate, 0, 2);

		if (in_rate == 0)
		{
			FirePowerAdjustRate = 0.0f;
		}
		else if (in_rate == 1)
		{
			FirePowerAdjustRate = 1.0f;
		}
		else if (in_rate == 2)
		{
			FirePowerAdjustRate = -1.0f;
		}
	}
}

void AAccelByteWarsPlayerPawn::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = InColor;
	OnRepNotify_Color();
}

void AAccelByteWarsPlayerPawn::Client_OnDestroyed_Implementation()
{
	IsDestroyed = true;

	ShowPowerLevelUITimer = 0.0f;
	UpdatePowerBarUI(0.01f);
}

void AAccelByteWarsPlayerPawn::OnRepNotify_Color()
{
	if (AccelByteWarsProceduralMesh == nullptr)
		return;

	AccelByteWarsProceduralMesh->UpdateColor(Color);
}

void AAccelByteWarsPlayerPawn::OnRepNotify_FirePowerLevel()
{
	// Nothing needed
}

void AAccelByteWarsPlayerPawn::OnRepNotify_RotationDirection()
{
	// Nothing needed
}

void AAccelByteWarsPlayerPawn::OnRepNotify_CurrentYaw()
{
	FRotator rot;
	rot.Roll = 0.0f;
	rot.Pitch = 0.0f;
	rot.Yaw = CurrentYaw;

	SetActorRotation(rot, ETeleportType::None);
}
