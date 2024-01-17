// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "OwnedCountWidgetEntry.h"

#include "Components/TextBlock.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

#include "Monetization/EntitlementsEssentials/UI/InventoryWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

void UOwnedCountWidgetEntry::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	SetVisibility(ESlateVisibility::Collapsed);
	if (EntitlementsSubsystem->GetIsQueryRunning())
	{
		Tb_OwnedCount->SetText(FText::FromString("..."));
	}
	else
	{
		ShowOwnedCount();
	}

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.AddWeakLambda(
		this,
		[this](const FOnlineError& Error, const TArray<UItemDataObject*> Entitlements)
		{
			ShowOwnedCount();
		});
}

void UOwnedCountWidgetEntry::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.RemoveAll(this);
}

void UOwnedCountWidgetEntry::ShowOwnedCount()
{
	if (UStoreItemListEntry* W_Parent = GetFirstOccurenceOuter<UStoreItemListEntry>())
	{
		UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
		if (!GameInstance)
		{
			return;
		}

		UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
		if (!BaseUIWidget)
		{
			return;
		}

		// Get item.
		const UItemDataObject* ItemData = W_Parent->GetItemData();
		if (!ItemData)
		{
			return;
		}

		FUniqueNetIdPtr UserId;
		if (const APlayerController* PC = GetOwningPlayer();
			const ULocalPlayer * LocalPlayer = PC->GetLocalPlayer())
		{
			UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
		}

		// Get entitlement.
		const UItemDataObject* EntitlementData = EntitlementsSubsystem->GetItemEntitlement(UserId, ItemData->Id);
		if (!EntitlementData)
		{
			return;
		}

		// Set owned count.
		FText Text;
		if (EntitlementData->bConsumable && EntitlementData->Count > 0)
		{
			const FString TextString = FString::Printf(TEXT("%d x"), EntitlementData->Count);
			Text = FText::FromString(TextString);
		}
		else if (!EntitlementData->bConsumable)
		{
			Text = TEXT_OWNED;
		}
		Tb_OwnedCount->SetText(Text);

		// Check if this widget is spawned in the inventory widget.
		bool bIsInInventory = false;
		if (Cast<UInventoryWidget>(BaseUIWidget->GetActiveWidgetOfStack(EBaseUIStackType::Menu, this)))
		{
			bIsInInventory = true;
		}

		// Highlight the entitlement item if it is equipped.
		if (bIsInInventory) 
		{
			const FString EquippedShip = ConvertShipDesignToItemId((ShipDesign)GameInstance->GetShipSelection());
			const FString EquippedPowerUp = ConvertPowerUpToItemId((PowerUpSelection)GameInstance->GetShipPowerUp());
			const bool bIsEquipped = (ItemData->Id.Equals(EquippedShip) || ItemData->Id.Equals(EquippedPowerUp));

			W_Parent->Execute_ToggleHighlight(W_Parent, bIsEquipped);
		}

		// Show the owned count if not empty.
		SetVisibility(Tb_OwnedCount->GetText().IsEmpty() || 
			(bIsInInventory && !EntitlementData->bConsumable) ? 
			ESlateVisibility::Collapsed : 
			ESlateVisibility::Visible);
	}
}
