// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerPawn.h"

// Sets default values
AAccelByteWarsPlayerPawn::AAccelByteWarsPlayerPawn()
{
	// Setup a do nothing root component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);
	SphereComponent->SetVisibility(false);
	RootComponent = SphereComponent;

	// Setup ship game play object
	GameplayObject = CreateDefaultSubobject<UAccelByteWarsGameplayObjectComponent>(TEXT("GameplayObject"));
	GameplayObject->Mass = 1.0f;
	GameplayObject->Radius = 0.5f;
	GameplayObject->ObjectType = EGameplayObjectType::SHIP;

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	bReplicates = true;
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
		UpdateShipLabelUI(DeltaTime);
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

void AAccelByteWarsPlayerPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Destroy all attached actors.
	if (EndPlayReason == EEndPlayReason::Type::Destroyed) 
	{
		TArray<AActor*> Actors;
		GetAttachedActors(Actors);
		for (AActor* Actor : Actors)
		{
			Actor->Destroy();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AAccelByteWarsPlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerPawn, RotationDirection);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, CurrentYaw);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, FirePowerLevel);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, PawnColor);
}

AAccelByteWarsMissile* AAccelByteWarsPlayerPawn::SpawnMissileInWorld(AActor* ActorOwner, FTransform InTransform, float InitialSpeed, FString BlueprintPath, bool ShouldReplicate)
{
	if (ActorOwner == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = ActorOwner;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* GenericClass = StaticLoadClass(AActor::StaticClass(), ActorOwner, *BlueprintPath);

	AAccelByteWarsMissile* NewActor = ActorOwner->GetWorld()->SpawnActor<AAccelByteWarsMissile>(GenericClass, InTransform, SpawnParameters);
	if (NewActor == nullptr)
		return nullptr;

	NewActor->Server_SetColor(PawnColor);
	NewActor->InitialSpeed = InitialSpeed;
	NewActor->SetVelocity();
	NewActor->SetInstigator(this);
	NewActor->SetReplicates(ShouldReplicate);

	return NewActor;
}

template<class T>
T* AAccelByteWarsPlayerPawn::SpawnBPActorInWorld(APawn* OwningPawn, const FVector Location, const FRotator Rotation, FString BlueprintPath, bool ShouldReplicate)
{
	if (OwningPawn == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwningPawn;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* GenericClass = StaticLoadClass(AActor::StaticClass(), OwningPawn, *BlueprintPath);
	if (GenericClass == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate generic class for: " + BlueprintPath);
		return nullptr;
	}

	T* NewActor = OwningPawn->GetWorld()->SpawnActor<T>(GenericClass, FTransform(Rotation, Location), SpawnParameters);
	if (NewActor == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate actor class for: " + BlueprintPath);
		return nullptr;
	}

	NewActor->SetInstigator(OwningPawn);
	NewActor->SetReplicates(ShouldReplicate);

	return NewActor;
}

void AAccelByteWarsPlayerPawn::Server_GetPlayerSelectedShip_Implementation(APlayerController* PlayerController, FLinearColor InColor)
{
	if (!HasAuthority())
	{
		return;
	}

	Client_GetPlayerSelectedShip(PlayerController, InColor);
}

void AAccelByteWarsPlayerPawn::Client_GetPlayerSelectedShip_Implementation(APlayerController* PlayerController, FLinearColor InColor)
{
	if (PlayerController == nullptr)
		return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
		return;

	int32 LocalPlayerNum = LocalPlayer->GetControllerId();

	// If online and primary player, broadcast event to configure player equipment based on online data.
	if (PlayerController->IsPrimaryPlayer() && OnMatchStarted.IsBound())
	{
		OnMatchStarted.Broadcast(this, PlayerController, InColor);
		return;
	}

	// If offline or not a primary player, configure the default player equipment.
	Server_SpawnPlayerShip((EShipDesign)EShipDesign::TRIANGLE);
	Server_RefreshSelectedPowerUp(EPowerUpSelection::NONE, 0);
	Server_SetColor(InColor);

	// Reset local equipment if a primary player.
	if (PlayerController->IsPrimaryPlayer()) 
	{
		if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
		{
			GameInstance->SetShipSelection((int32)EShipDesign::TRIANGLE);
			GameInstance->SetShipPowerUp((int32)EPowerUpSelection::NONE);
		}
		if (OnPlayerEquipmentLoaded.IsBound())
		{
			OnPlayerEquipmentLoaded.Broadcast(this, EShipDesign::TRIANGLE, EPowerUpSelection::NONE);
		}
	}
}

void AAccelByteWarsPlayerPawn::Server_SpawnPlayerShip_Implementation(const EShipDesign SelectedShipDesign)
{
	if (!HasAuthority()) 
	{
		return;
	}

	switch (SelectedShipDesign)
	{
		case EShipDesign::TRIANGLE:
		{
			PlayerShip = SpawnBPActorInWorld<APlayerShipTriangle>(this, GetActorLocation(), GetActorRotation(), PlayerShipBlueprintPaths[(int8)SelectedShipDesign], true);
		}
		break;
		case EShipDesign::D:
		{
			PlayerShip = SpawnBPActorInWorld<APlayerShipD>(this, GetActorLocation(), GetActorRotation(), PlayerShipBlueprintPaths[(int8)SelectedShipDesign], true);
		}
		break;
		case EShipDesign::DOUBLE_TRIANGLE:
		{
			PlayerShip = SpawnBPActorInWorld<APlayerShipDoubleTriangle>(this, GetActorLocation(), GetActorRotation(), PlayerShipBlueprintPaths[(int8)SelectedShipDesign], true);
		}
		break;
		case EShipDesign::GLOW_XTRA:
		{
			PlayerShip = SpawnBPActorInWorld<APlayerShipGlowXtra>(this, GetActorLocation(), GetActorRotation(), PlayerShipBlueprintPaths[(int8)SelectedShipDesign], true);
		}
		break;
		case EShipDesign::WHITE_STAR:
		{
			PlayerShip = SpawnBPActorInWorld<APlayerShipWhiteStar>(this, GetActorLocation(), GetActorRotation(), PlayerShipBlueprintPaths[(int8)SelectedShipDesign], true);
		}
		break;
	}

	if (PlayerShip != nullptr)
		PlayerShip->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);

	if (PlayerShip->AccelByteWarsProceduralMesh)
		PlayerShip->AccelByteWarsProceduralMesh->SetupAttachment(SphereComponent);
}

void AAccelByteWarsPlayerPawn::ValidateActivatePowerUp(const EPowerUpSelection SelectedPowerUp)
{
	// If no power up selected, abort.
	if (SelectedPowerUp == EPowerUpSelection::NONE) 
	{
		return;
	}

	// If online, power up can only be activated if the player is entitled to activate the power up.
	if (OnValidateActivatePowerUp.IsBound())
	{
		OnValidateActivatePowerUp.Broadcast(this, SelectedPowerUp);
		return;
	}

	// If offline, power up can be activated without validation.
	Server_ActivatePowerUp(SelectedPowerUp);
}

void AAccelByteWarsPlayerPawn::Server_RefreshSelectedPowerUp_Implementation(const EPowerUpSelection SelectedPowerUp, const int32 PowerUpCount)
{
	if (!HasAuthority())
	{
		return;
	}

	AAccelByteWarsInGameGameMode* ABInGameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!ABInGameMode) 
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController())) 
	{
		ABInGameMode->OnRefreshPlayerSelectedPowerUp(PC, SelectedPowerUp, PowerUpCount);
	}
}

void AAccelByteWarsPlayerPawn::Server_ActivatePowerUp_Implementation(const EPowerUpSelection SelectedPowerUp)
{
	if (!HasAuthority())
	{
		return;
	}

	// In case multiple power ups are used at once (this shouldn't happen, but if allowed in the future, this will deal with it)
	if (PowerUp != nullptr) 
	{
		// Do not create a new shield if already exist.
		const APowerUpByteShield* LastShield = Cast<APowerUpByteShield>(PowerUp);
		if (LastShield && LastShield->IsShieldActive)
		{
			return;
		}

		PowerUp->DestroyPowerUp();
	}

	switch (SelectedPowerUp)
	{
		case EPowerUpSelection::NONE:
		{
			// No power up equipped, do nothing and return;
			return;
		}
		break;
		case EPowerUpSelection::BYTE_BOMB:
		{
			PowerUp = SpawnBPActorInWorld<APowerUpByteBomb>(this, GetActorLocation(), GetActorRotation(), PlayerPowerUpBlueprintPaths[(int8)SelectedPowerUp], true);

			APowerUpByteBomb* ByteBomb = Cast<APowerUpByteBomb>(PowerUp);
			if (ByteBomb == nullptr)
				return;

			ByteBomb->InitiateExplosion();
			PowerUp = nullptr;
		}
		break;
		case EPowerUpSelection::BYTE_SHIELD:
		{
			PowerUp = SpawnBPActorInWorld<APowerUpByteShield>(this, GetActorLocation(), GetActorRotation(), PlayerPowerUpBlueprintPaths[(int8)SelectedPowerUp], true);

			APowerUpByteShield* ByteShield = Cast<APowerUpByteShield>(PowerUp);
			if (ByteShield == nullptr)
				return;

			PowerUp->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
			ByteShield->SetShieldColor(PawnColor);
		}
		break;
		case EPowerUpSelection::WORM_HOLE:
		{
			PowerUp = SpawnBPActorInWorld<APowerUpWormHole>(this, GetActorLocation(), GetActorRotation(), PlayerPowerUpBlueprintPaths[(int8)SelectedPowerUp], true);

			APowerUpWormHole* WormHole = Cast<APowerUpWormHole>(PowerUp);
			if (WormHole == nullptr)
				return;

			PowerUp->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
			WormHole->SetWormHoleColor(PawnColor);
			WormHole->Server_InitiateWormHoleGenerator();
		}
		break;
		case EPowerUpSelection::SPLIT_MISSILE:
		{
			// Do not use PowerUp if a missile is not in use
			if (FiredMissile == nullptr)
				return;

			PowerUp = SpawnBPActorInWorld<APowerUpSplitMissile>(this, GetActorLocation(), GetActorRotation(), PlayerPowerUpBlueprintPaths[(int8)SelectedPowerUp], true);

			APowerUpSplitMissile* SplitMissiles = Cast<APowerUpSplitMissile>(PowerUp);
			if (SplitMissiles == nullptr)
				return;

			SplitMissiles->Server_FireMissiles(this, FiredMissile->GetActorTransform(), FirePowerLevel, MinMissileSpeed, MaxMissileSpeed, PawnColor, FiredMissileBlueprintPath, FiredMissileTrailBlueprintPath);
			FireMissileFx(SplitMissiles->ParentMissileTransform);
		}
		break;
	}	
}

void AAccelByteWarsPlayerPawn::Server_FireMissile_Implementation()
{
	if (!HasAuthority() || !ShouldFire())
	{
		return;
	}

	// Calculate missile spawn location
	FTransform SpawnTransform = CalculateWhereToSpawnMissile();

	// Clamp initial missile speed
	float ClampedInitialSpeed = UKismetMathLibrary::MapRangeClamped(FirePowerLevel, 0.0f, 1.0f, MinMissileSpeed, MaxMissileSpeed);

	// Spawn missile actor
	FiredMissile = SpawnBPActorInWorld<AAccelByteWarsMissile>(this, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), FiredMissileBlueprintPath, true);
	if (FiredMissile == nullptr)
		return;

	FiredMissile->Server_SetColor(PawnColor);
	FiredMissile->InitialSpeed = ClampedInitialSpeed;
	FiredMissile->SetVelocity();

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
	MissileTrail = SpawnBPActorInWorld<AAccelByteWarsMissileTrail>(this, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), FiredMissileTrailBlueprintPath, true);
	if (MissileTrail == nullptr)
		return;

	MissileTrail->Server_SetColor(PawnColor);

	// Attach new trail to missile actor
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	MissileTrail->AttachToActor(FiredMissile, AttachmentRules);
}

FTransform AAccelByteWarsPlayerPawn::CalculateWhereToSpawnMissile()
{
	FTransform ActorTransform = GetActorTransform();
	FVector RightVector = ActorTransform.GetRotation().GetRightVector();
	FVector DesiredLocation = (RightVector * 100.0f) + ActorTransform.GetLocation();

	FTransform OutTransform;
	OutTransform.SetLocation(DesiredLocation);
	OutTransform.SetRotation(ActorTransform.GetRotation());
	OutTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	return OutTransform;
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

	FirePowerLevel += FMath::Clamp(Rate, -1, 1);
}

void AAccelByteWarsPlayerPawn::OnPlayerInputThisFrame()
{
	if (IsDestroyed == false)
		ShowPowerLevelUITimer = 1.0f;
}

void AAccelByteWarsPlayerPawn::Server_RotatePawn_Implementation(int Rate)
{
	if (!HasAuthority()) 
	{
		return;
	}

	Client_RotatePawn_Implementation(Rate);
}

UAccelByteWarsProceduralMeshComponent* AAccelByteWarsPlayerPawn::GetPlayerShipMesh()
{
	if (PlayerShip == nullptr)
		return nullptr;

	if (PlayerShip->AccelByteWarsProceduralMesh == nullptr)
		return nullptr;

	return PlayerShip->AccelByteWarsProceduralMesh;
}

void AAccelByteWarsPlayerPawn::Client_RotatePawn_Implementation(int Rate)
{
	if (HasAuthority())
	{
		int InRate = FMath::Clamp(Rate, 0, 2);

		RotationDirection = InRate;
		OnRepNotify_RotationDirection();
	}
}

void AAccelByteWarsPlayerPawn::Server_AdjustFirePower_Implementation(FVector PlayerPosition, int Rate)
{
	if (!HasAuthority())
	{
		return;
	}

	int InRate = FMath::Clamp(Rate, 0, 2);

	if (InRate == 0)
	{
		FirePowerAdjustRate = 0.0f;
	}
	else if (InRate == 1)
	{
		FirePowerAdjustRate = 1.0f;
	}
	else if (InRate == 2)
	{
		FirePowerAdjustRate = -1.0f;
	}

	Client_AdjustFirePower(PlayerPosition, PawnColor);
}

void AAccelByteWarsPlayerPawn::Client_AdjustFirePower_Implementation(FVector PlayerPosition, FLinearColor InColor)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
		return;

	AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (PlayerController == nullptr)
		return;

	if (ABPlayerController->ABPlayerHUD == nullptr)
		return;

	FVector2D ScreenLocation = FVector2D::ZeroVector;
	PlayerController->ProjectWorldLocationToScreen(PlayerPosition, ScreenLocation, false);

	ABPlayerController->ABPlayerHUD->UpdatePowerBarUI(ScreenLocation, InColor);
}

void AAccelByteWarsPlayerPawn::Server_SetColor_Implementation(FLinearColor InColor)
{
	if (!HasAuthority())
	{
		return;
	}

	PawnColor = InColor;

	if (PlayerShip != nullptr)
		PlayerShip->SetShipColor(PawnColor);

	OnRepNotify_Color();
}

void AAccelByteWarsPlayerPawn::Client_OnDestroyed_Implementation()
{
	IsDestroyed = true;

	ShowPowerLevelUITimer = 0.0f;
	ShowShipLabelUITimer = 0.0f;

	UpdatePowerBarUI(0.01f);
	UpdateShipLabelUI(0.01f);
}

void AAccelByteWarsPlayerPawn::OnRepNotify_Color()
{
	if (PlayerShip != nullptr)
		PlayerShip->SetShipColor(PawnColor);
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
	FRotator Rotation;
	Rotation.Roll = 0.0f;
	Rotation.Pitch = 0.0f;
	Rotation.Yaw = CurrentYaw;

	SetActorRotation(Rotation, ETeleportType::None);
}