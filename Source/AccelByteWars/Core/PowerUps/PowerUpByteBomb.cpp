// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpByteBomb.h"

#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerPawn.h"
#include "Components/SphereComponent.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Kismet/GameplayStatics.h"

APowerUpByteBomb::APowerUpByteBomb()
{
	// Setup a do nothing root component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BombSphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;

	// Setup audio component
	BombAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BombAudioComponent"));
	BombAudioComponent->SetupAttachment(RootComponent);
	BombAudioComponent->OnAudioFinished.AddUniqueDynamic(this, &ThisClass::DestroyItem);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	AActor::SetReplicateMovement(false);

	if (!IsRunningGame() || !GetWorld() || (GIsEditor && GetWorld()->WorldType != EWorldType::PIE))
	{
		return;
	}

	// check if there are missiles
	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);
	if (FoundMissiles.IsEmpty())
	{
		APowerUpByteBomb::DestroyItem();
		return;
	}
}

void APowerUpByteBomb::BeginPlay()
{
	Super::BeginPlay();
}

void APowerUpByteBomb::OnUse()
{
	IInGameItemInterface::OnUse();

	if (!HasAuthority())
	{
		return;
	}

	// check if there are enemy's missiles
	bool bEnemyMissileFound = false;
	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);
	for (AActor* Actor : FoundMissiles)
	{
		if (const AAccelByteWarsMissile* Missile = Cast<AAccelByteWarsMissile>(Actor);
			Missile && GetTeamIdFromPawn(Missile->GetOwner()) != GetTeamIdFromPawn(GetOwner()))
		{
			bEnemyMissileFound = true;
			break;
		}
	}
	if (!bEnemyMissileFound)
	{
		APowerUpByteBomb::DestroyItem();
		return;
	}

	ShakeCamera();
	DestroyAllEnemyMissiles(GetTeamIdFromPawn(GetOwner()));
}

void APowerUpByteBomb::OnEquip()
{
	IInGameItemInterface::OnEquip();
}

void APowerUpByteBomb::DestroyItem()
{
	if (BombAudioComponent) 
	{
		BombAudioComponent->Stop();
	}

	Destroy();
}

void APowerUpByteBomb::ShakeCamera()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
			return;

		constexpr float Scale = 0.2f;
		PlayerController->ClientStartCameraShake(ShakeClass, Scale);
	}
}

void APowerUpByteBomb::DestroyAllEnemyMissiles(int32 TeamId)
{
	TArray<AActor*> FoundMissiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAccelByteWarsMissile::StaticClass(), FoundMissiles);

	for (AActor* Actor : FoundMissiles)
	{
		if (AAccelByteWarsMissile* Missile = Cast<AAccelByteWarsMissile>(Actor);
			Missile && GetTeamIdFromPawn(Missile->Owner) != TeamId)
		{
			// Broadcast entity destroyed event
			NotifyMissileDestroyed(Missile);

			Missile->DestroyByPowerUp();
		}
	}
}

int32 APowerUpByteBomb::GetTeamIdFromPawn(const AActor* Actor) const
{
	const AAccelByteWarsPlayerPawn* MissileOwner = Cast<AAccelByteWarsPlayerPawn>(Actor);
	if (MissileOwner == nullptr)
	{
		return INDEX_NONE;
	}

	const AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(MissileOwner->GetController());
	if (ABPlayerController == nullptr)
	{
		return INDEX_NONE;
	}

	if (ABPlayerController->PlayerState == nullptr)
	{
		return INDEX_NONE;
	}

	const AAccelByteWarsPlayerState* ABPlayerState = static_cast<AAccelByteWarsPlayerState*>(ABPlayerController->PlayerState);
	if (ABPlayerState == nullptr)
	{
		return INDEX_NONE;
	}

	return ABPlayerState->TeamId;
}

void APowerUpByteBomb::NotifyMissileDestroyed(const AActor* MissileToBeDestroyed)
{
	if (!MissileToBeDestroyed)
	{
		UE_LOG_IN_GAME_ITEM(Warning, "MissileToBeDestroyed is invalid. Operation canceled.")
		return;
	}

	if (!GetOwner())
	{
		UE_LOG_IN_GAME_ITEM(Warning, "MissileToBeDestroyed's Owner is invalid. Operation canceled.")
		return;
	}

	const UPlayer* Player = GetNetOwningPlayer();
	if (!Player)
	{
		UE_LOG_IN_GAME_ITEM(Warning, "OwningPlayer is invalid. Operation canceled.")
		return;
	}

	const APlayerController* PlayerController = Player->GetPlayerController(GetWorld());
	if (!PlayerController)
	{
		UE_LOG_IN_GAME_ITEM(Warning, "OwningPlayer's PlayerController is invalid. Operation canceled.")
		return;
	}

	const APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();
	if (!PlayerState)
	{
		UE_LOG_IN_GAME_ITEM(Warning, "PlayerState of OwningPlayer is invalid. Operation canceled.")
		return;
	}

	// Missile destroyed by power up, broadcast event for the missile.
	AAccelByteWarsInGameGameMode::OnEntityDestroyedDelegates.Broadcast(
		ENTITY_TYPE_MISSILE,
		nullptr,
		MissileToBeDestroyed->GetName(),
		MissileToBeDestroyed->GetActorLocation(),
		ENTITY_DESTROYED_TYPE_HIT_POWERUP,
		AccelByteWarsUtility::FormatEntityDeathSource(ENTITY_TYPE_POWERUP, AccelByteWarsUtility::GenerateActorEntityId(GetOwner()))
	);
}
