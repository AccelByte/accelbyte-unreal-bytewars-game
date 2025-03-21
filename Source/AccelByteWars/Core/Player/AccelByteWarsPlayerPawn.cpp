// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerPawn.h"

#include "AccelByteWarsPlayerController.h"
#include "Components/SphereComponent.h"
#include "Core/Actor/AccelByteWarsMissile.h"
#include "Core/Actor/AccelByteWarsMissileTrail.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/Ships/PlayerShipBase.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"

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
	AActor::SetReplicateMovement(true);
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

void AAccelByteWarsPlayerPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UpdateSkin();
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
T* AAccelByteWarsPlayerPawn::SpawnActorInWorld(
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

void AAccelByteWarsPlayerPawn::UpdateSkin()
{
	if (!HasAuthority())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerController->PlayerState);
	if (!AbPlayerState)
	{
		return;
	}

	// get skin item
	const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAsset(AbPlayerState->GetEquippedItemId(EItemType::Skin));
	if (!ItemDataAsset)
	{
		// none set, use default
		ItemDataAsset = DefaultSkin;
	}

	PlayerShip = SpawnActorInWorld<APlayerShipBase>(
		this,
		GetActorLocation(),
		GetActorRotation(),
		ItemDataAsset->Actor,
		true);
	if (PlayerShip)
	{
		PlayerShip->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	if (PlayerShip->AccelByteWarsProceduralMesh)
	{
		PlayerShip->AccelByteWarsProceduralMesh->SetupAttachment(SphereComponent);
	}
}

void AAccelByteWarsPlayerPawn::ClientPowerUpActivated_Implementation()
{
	// only run on owning player
	if (GetLocalRole() != ENetRole::ROLE_Authority && GetRemoteRole() != ENetRole::ROLE_Authority)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	// get item
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerController->PlayerState);
	if (!AbPlayerState)
	{
		return;
	}
	const UInGameItemDataAsset* Item = UInGameItemUtility::GetItemDataAsset(AbPlayerState->GetEquippedItemId(EItemType::PowerUp));
	if (!Item)
	{
		return;
	}

	if (OnPowerUpActivatedDelegates.IsBound())
	{
		OnPowerUpActivatedDelegates.Broadcast(PlayerController, Item->Id);
	}

	// Get Local player index
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	const int32 PlayerIndex = LocalPlayer->GetControllerId();

	// Update stored power up quantity
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	GameInstance->ModifyEquippedItemCountByInGameItemId(PlayerIndex, Item->Id, -1);
}

void AAccelByteWarsPlayerPawn::ServerActivatePowerUp_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	// only allowed one power up at a time
	if (GetActivePowerUp())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerController->PlayerState);
	if (!AbPlayerState)
	{
		return;
	}

	// get item
	const UInGameItemDataAsset* Item = UInGameItemUtility::GetItemDataAsset(AbPlayerState->GetEquippedItemId(EItemType::PowerUp));
	if (!Item || !Item->Actor)
	{
		return;
	}

	// create item
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* PowerUp = GetWorld()->SpawnActor<AActor>(Item->Actor, FTransform(GetActorRotation(), GetActorLocation()), SpawnParameters);
	if (!PowerUp)
	{
		return;
	}
	PowerUp->SetInstigator(this);

	// decrease use num
	AbPlayerState->DecreaseEquippedItemCount(EItemType::PowerUp);

	// activate item
	IInGameItemInterface* ItemInterface = Cast<IInGameItemInterface>(PowerUp);
	if (!ItemInterface)
	{
		return;
	}
	ItemInterface->OnUse();

	// Notify other
	constexpr int UsedAmount = 1;
	const int TotalAmount = AbPlayerState->GetEquippedItem(EItemType::PowerUp).Count;
	if (OnPowerUpUsedServerDelegates.IsBound()) 
	{
		OnPowerUpUsedServerDelegates.Broadcast(
			AbPlayerState->GetUniqueId().GetUniqueNetId(),
			GetName(),
			UsedAmount,
			TotalAmount);
	}
	ClientPowerUpActivated();
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
	FiredMissile = SpawnActorInWorld<AAccelByteWarsMissile>(this, SpawnTransform.GetLocation(), SpawnTransform.Rotator(), MissileActor, true);
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
	MissileTrail = SpawnActorInWorld<AAccelByteWarsMissileTrail>(
		this,
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		MissileTrailActor,
		true);
	if (MissileTrail == nullptr)
		return;

	MissileTrail->Server_SetColor(PawnColor);

	// Attach new trail to missile actor
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	MissileTrail->AttachToActor(FiredMissile, AttachmentRules);

	// Reduce ship glow if MissilesFired limit reached
	UpdateShipGlow();
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

void AAccelByteWarsPlayerPawn::UpdateShipGlow()
{
	if (PlayerShip)
	{
		const float Modifier = ShouldFire() ? 1.0f : 0.5f;
		PlayerShip->SetGlowModifier(Modifier);
	}
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

void AAccelByteWarsPlayerPawn::SetColor(FLinearColor InColor)
{
	if (!HasAuthority())
	{
		return;
	}

	PawnColor = InColor;

	if (PlayerShip != nullptr)
	{
		PlayerShip->SetShipColor(PawnColor);
	}

	// Trigger RepNotify manually on P2P host for itself
	if (!IsRunningDedicatedServer())
	{
		OnRepNotify_Color();
	}
}

IInGameItemInterface* AAccelByteWarsPlayerPawn::GetActivePowerUp() const
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (IInGameItemInterface* ItemInterface = Cast<IInGameItemInterface>(AttachedActor))
		{
			if (ItemInterface->GetType() == EItemType::PowerUp)
			{
				return ItemInterface;
			}
		}
	}

	return nullptr;
}

void AAccelByteWarsPlayerPawn::Client_OnDestroyed_Implementation()
{
	IsDestroyed = true;

	ShowPowerLevelUITimer = 0.0f;
	ShowShipLabelUITimer = 0.0f;

	UpdatePowerBarUI(0.01f);
	UpdateShipLabelUI(0.01f);
	UpdateShipGlow();
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
