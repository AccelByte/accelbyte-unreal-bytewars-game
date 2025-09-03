// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerPawn.h"

#include "AccelByteWarsPlayerController.h"
#include "AccelByteWarsPlayerState.h"
#include "Components/SphereComponent.h"
#include "Core/Actor/AccelByteWarsMissile.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/Ships/PlayerShipBase.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "AbilitySystemComponent.h"
#include "AccelByteWarsAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

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
		// Adjust fire power
		if (FirePowerAdjustRate != 0.0f)
		{
			float a = DeltaTime * FirePowerAdjustRate * ConstFirePowerAdjustRate;
			float b = FirePowerLevel + a;

			FirePowerLevel = FMath::Clamp(b, 0.0f, 1.0f);
		}

		if (RotationDirection != 0)
		{
			CurrentYaw -= (ConstRotateRate * RotationDirection);
			OnRepNotify_CurrentYaw();
		}
	}
}

void AAccelByteWarsPlayerPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize GAS on the server when possessed
	if (AAccelByteWarsPlayerState* PS = GetPlayerState<AAccelByteWarsPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, this);
		}
	}

	UpdateSkin();
}

void AAccelByteWarsPlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize GAS on the client when PlayerState replicates
	if (AAccelByteWarsPlayerState* PS = GetPlayerState<AAccelByteWarsPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, this);
		}
	}
}

UAbilitySystemComponent* AAccelByteWarsPlayerPawn::GetAbilitySystemComponent() const
{
	if (AAccelByteWarsPlayerState* PS = GetPlayerState<AAccelByteWarsPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AAccelByteWarsPlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerPawn, RotationDirection);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, CurrentYaw);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, FirePowerLevel);
	DOREPLIFETIME(AAccelByteWarsPlayerPawn, PawnColor);
}

template <class T>
T* AAccelByteWarsPlayerPawn::SpawnActorInWorld(
	AActor* Owner,
	const FVector Location,
	const FRotator Rotation,
	TSubclassOf<AActor> ActorClass,
	bool ShouldReplicate)
{
	if (Owner == nullptr)
		return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	T* NewActor = Owner->GetWorld()->SpawnActor<T>(ActorClass, FTransform(Rotation, Location), SpawnParameters);
	if (NewActor == nullptr)
	{
		LOG_TO_CONSOLE("Failed to generate actor class for: " + ActorClass->GetName());
		return nullptr;
	}

	if (APawn* OwningPawn = Cast<APawn>(Owner))
	{
		NewActor->SetInstigator(OwningPawn);
	}
	NewActor->SetReplicates(ShouldReplicate);

	return NewActor;
}

void AAccelByteWarsPlayerPawn::UpdateSkin()
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: Can be bot
	// const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	// if (!PlayerController)
	// {
	// 	return;
	// }
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(Controller->PlayerState);
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
	PowerUp->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));

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

	// Update last missile fire time for cooldown
	LastMissileFireTime = GetWorld()->GetTimeSeconds();

	// Increment missiles fired
	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(GetPlayerState());
	if (ABPlayerState == nullptr)
		return;

	ABPlayerState->MissilesFired++;

	// Apply buff if any
	FiredMissile->SetActorScale3D(FVector::One() * ABPlayerState->GetAttributeSet()->MissileSizeMultiplier.GetCurrentValue());
	FiredMissile->AccelByteWarsGameplayObjectComponent->Radius *= ABPlayerState->GetAttributeSet()->MissileSizeMultiplier.GetCurrentValue();

	// Play sound for missile fire
	FireMissileFx(SpawnTransform);

	// Reduce ship glow if MissilesFired limit reached
	UpdateShipGlow();

	// Track this missile and clean up when it gets destroyed
	TrackedMissiles.Add(FiredMissile);
	// Bind to missile's OnDestroyed to remove from tracking when it dies by itself
	FiredMissile->OnDestroyed.AddDynamic(this, &AAccelByteWarsPlayerPawn::OnTrackedMissileDestroyed);
}

void AAccelByteWarsPlayerPawn::Destroyed()
{
	Super::Destroyed();

	// Server-side: expire all missiles owned by this pawn when the pawn is destroyed
	if (HasAuthority())
	{
		AAccelByteWarsGameMode* GameMode = Cast<AAccelByteWarsGameMode>(GetWorld()->GetAuthGameMode());

		// Copy to local array to avoid modifying set while iterating
		TArray<TWeakObjectPtr<AAccelByteWarsMissile>> ToExpire;
		ToExpire.Reserve(TrackedMissiles.Num());
		for (const TWeakObjectPtr<AAccelByteWarsMissile>& WeakMissile : TrackedMissiles)
		{
			ToExpire.Add(WeakMissile);
		}

		for (const TWeakObjectPtr<AAccelByteWarsMissile>& WeakMissile : ToExpire)
		{
			if (AAccelByteWarsMissile* Missile = WeakMissile.Get())
			{
				// Unbind to avoid callbacks into this pawn during teardown
				Missile->OnDestroyed.RemoveAll(this);
				Missile->TreatMissileAsExpired(GameMode);
			}
		}

		TrackedMissiles.Empty();
	}
}

void AAccelByteWarsPlayerPawn::OnTrackedMissileDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority() || !DestroyedActor)
	{
		return;
	}

	if (AAccelByteWarsMissile* Missile = Cast<AAccelByteWarsMissile>(DestroyedActor))
	{
		TrackedMissiles.Remove(Missile);
	}
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

	// Check missile limit
	if (ABPlayerState->MissilesFired >= ABInGameState->GameSetup.FiredMissilesLimit)
		return false;

	// Check cooldown timer
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastMissileFireTime < MissileCooldownDuration)
		return false;

	return true;
}

void AAccelByteWarsPlayerPawn::ResetMissileCooldown()
{
	LastMissileFireTime = 0.0f;
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
		RotationDirection = FMath::Sign(Rate);
		OnRepNotify_RotationDirection();
	}
}

void AAccelByteWarsPlayerPawn::Server_AdjustFirePower_Implementation(FVector PlayerPosition, int Rate)
{
	if (!HasAuthority())
	{
		return;
	}

	FirePowerAdjustRate = FMath::Sign(Rate);
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

	// Reset missile cooldown when player is destroyed
	ResetMissileCooldown();

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
