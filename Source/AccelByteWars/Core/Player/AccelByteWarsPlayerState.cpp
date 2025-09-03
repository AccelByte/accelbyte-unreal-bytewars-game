// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerState.h"

#include "AccelByteWarsPlayerController.h"
#include "AccelByteWarsPlayerPawn.h"
#include "AccelByteWarsAttributeSet.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

AAccelByteWarsPlayerState::AAccelByteWarsPlayerState()
{
	// Create GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	AttributeSet = CreateDefaultSubobject<UAccelByteWarsAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAccelByteWarsPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAccelByteWarsPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Set NetUpdateFrequency to a higher value for responsive GAS replication
	SetNetUpdateFrequency(100.0f);

	if (const AAccelByteWarsPlayerController* PlayerController = Cast<AAccelByteWarsPlayerController>(GetOwningController()))
	{
		if (HasLocalNetOwner())
		{
			PlayerController->LoadingPlayerAssignment();
		}
	}

	// Listen for GameplayEffect applications to handle tag-based effects
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &AAccelByteWarsPlayerState::OnGameplayEffectApplied);
		// Also listen for when active effects are added (for HUD notifications)
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &AAccelByteWarsPlayerState::OnActiveGameplayEffectAdded);
		// Listen for when active effects are removed (for HUD cleanup)
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &AAccelByteWarsPlayerState::OnActiveGameplayEffectRemoved);
	}
}

void AAccelByteWarsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerState, AvatarURL);
	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamColor);
	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamId);
	DOREPLIFETIME(AAccelByteWarsPlayerState, MissilesFired);
	DOREPLIFETIME(AAccelByteWarsPlayerState, KillCount);
	DOREPLIFETIME(AAccelByteWarsPlayerState, Deaths);
	DOREPLIFETIME(AAccelByteWarsPlayerState, NumLivesLeft);
	DOREPLIFETIME(AAccelByteWarsPlayerState, bPendingTeamAssignment);
	DOREPLIFETIME(AAccelByteWarsPlayerState, NumKilledAttemptInSingleLifetime);
	DOREPLIFETIME(AAccelByteWarsPlayerState, EquippedItems);
}

void AAccelByteWarsPlayerState::RepNotify_PendingTeamAssignment()
{
	if (const AAccelByteWarsPlayerController* PlayerController = Cast<AAccelByteWarsPlayerController>(GetOwningController()))
	{
		if (HasLocalNetOwner())
		{
			PlayerController->LoadingPlayerAssignment();
		}
	}
}

void AAccelByteWarsPlayerState::RepNotify_EquippedItemsChanged()
{
	NotifyGameState();
}

FString AAccelByteWarsPlayerState::GetEquippedItemId(const EItemType ItemType, const bool bForce)
{
	return GetEquippedItem(ItemType, bForce).ItemId;
}

FEquippedItem AAccelByteWarsPlayerState::GetEquippedItem(const EItemType ItemType, const bool bForce)
{
	for (const FEquippedItem& EquippedItem : EquippedItems)
	{
		if (EquippedItem.ItemType == ItemType)
		{
			if (bForce || EquippedItem.Count > 0)
			{
				return EquippedItem;
			}
		}
	}

	return FEquippedItem();
}

void AAccelByteWarsPlayerState::DecreaseEquippedItemCount(const EItemType ItemType, int32 DecreaseBy)
{
	for (FEquippedItem& EquippedItem : EquippedItems)
	{
		if (EquippedItem.ItemType == ItemType)
		{
			EquippedItem.Count -= DecreaseBy;
		}
	}

	if (!IsRunningDedicatedServer())
	{
		NotifyGameState();
	}
}

void AAccelByteWarsPlayerState::NotifyGameState() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		return;
	}
	const AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		return;
	}
	AbGameState->OnPowerUpChanged.Broadcast();
}

void AAccelByteWarsPlayerState::ClientRetrieveEquippedItems_Implementation()
{
	// get equipped item from game instance
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UAccelByteWarsGameInstance* ABGameInstance = Cast<UAccelByteWarsGameInstance>(GameInstance);
	if (!ABGameInstance)
	{
		return;
	}

	const TArray<FEquippedItem>* Equipments = ABGameInstance->GetEquippedItems(
		GetPlayerController()->GetLocalPlayer()->GetControllerId());
	if (!Equipments)
	{
		return;
	}

	ServerUpdateEquippedItems(*Equipments);
}

void AAccelByteWarsPlayerState::ServerUpdateEquippedItems_Implementation(const TArray<FEquippedItem>& Items)
{
	if (!HasAuthority())
	{
		return;
	}

	EquippedItems.Empty();
	for (const FEquippedItem& Item : Items)
	{
		if (const UInGameItemDataAsset* InGameItemDataAsset = UInGameItemUtility::GetItemDataAsset(Item.ItemId))
		{
			EquippedItems.Add({ InGameItemDataAsset->Type, InGameItemDataAsset->Id, Item.Count });
		}
	}

	// Give modules a chance to verify items
	OnEquippedItemsLoadedDelegate.Broadcast(this);

	// If this is a P2P host, trigger manually
	if (!IsRunningDedicatedServer())
	{
		RepNotify_EquippedItemsChanged();
	}

	// update pawn
	if (APawn* Pawn = GetPawn())
	{
		AAccelByteWarsPlayerPawn* AbPawn = Cast<AAccelByteWarsPlayerPawn>(Pawn);
		if (!ensure(AbPawn))
		{
			return;
		}
		AbPawn->UpdateSkin();
	}
}

void AAccelByteWarsPlayerState::OnGameplayEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle)
{
	// Check for tag-based effects
	const UAssetTagsGameplayEffectComponent* TagEffectComponent = EffectSpec.Def->FindComponent<UAssetTagsGameplayEffectComponent>();
	if (!TagEffectComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAccelByteWarsPlayerState::OnGameplayEffectApplied UAssetTagsGameplayEffectComponent not found in the EffectSpec"))
		return;
	}
	const FGameplayTagContainer& EffectTags = TagEffectComponent->GetConfiguredAssetTagChanges().CombinedTags;

	if (EffectTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.LivesUp"))))
	{
		if (AAccelByteWarsInGameGameMode* ABGameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			// Use the game mode's increment function if it exists
			ABGameMode->IncrementPlayerLife(this, 1);
		}
	}
	if (EffectTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.ScoreAdd100"))))
	{
		if (AAccelByteWarsInGameGameMode* ABGameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			ABGameMode->AddPlayerScore(this, 100, false);
		}
	}

	// Always notify team changes for all gameplay effects (for HUD updates)
	if (AAccelByteWarsInGameGameState* ABInGameGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(this)))
	{
		ABInGameGameState->OnNotify_Teams();
	}
}

void AAccelByteWarsPlayerState::OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle)
{
	if (HasAuthority() && ASC)
	{
		if (AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(this)))
		{
			GameState->MulticastRefreshHUDGameplayEffects();
		}
	}
}

void AAccelByteWarsPlayerState::OnActiveGameplayEffectRemoved(const FActiveGameplayEffect& RemovedEffect)
{
	// Execute GameplayCue to notify all clients about HUD changes
	if (HasAuthority() && AbilitySystemComponent)
	{
		if (AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(this)))
		{
			GameState->MulticastRefreshHUDGameplayEffects();
		}
	}
}
