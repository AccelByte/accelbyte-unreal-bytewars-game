// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Player/AccelByteWarsPlayerState.h"

#include "AccelByteWarsPlayerController.h"
#include "AccelByteWarsPlayerPawn.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Net/UnrealNetwork.h"

void AAccelByteWarsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerState, AvatarURL);
	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamColor);
	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamId);
	DOREPLIFETIME(AAccelByteWarsPlayerState, MissilesFired);
	DOREPLIFETIME(AAccelByteWarsPlayerState, KillCount);
	DOREPLIFETIME(AAccelByteWarsPlayerState, NumLivesLeft);
	DOREPLIFETIME(AAccelByteWarsPlayerState, bPendingTeamAssignment);
	DOREPLIFETIME(AAccelByteWarsPlayerState, NumKilledAttemptInSingleLifetime);
	DOREPLIFETIME(AAccelByteWarsPlayerState, EquippedItems);
}

void AAccelByteWarsPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (const AAccelByteWarsPlayerController* PlayerController = Cast<AAccelByteWarsPlayerController>(C))
	{
		if (HasLocalNetOwner())
		{
			PlayerController->LoadingPlayerAssignment();
		}
	}
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
			EquippedItems.Add({InGameItemDataAsset->Type, InGameItemDataAsset->Id, Item.Count});
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
