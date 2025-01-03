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
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget.h"

void UOwnedCountWidgetEntry::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	RetrieveEntitlementWithForceRequest(false);

	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		RetrieveEntitlementWithForceRequest(true);
	});
}

void UOwnedCountWidgetEntry::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.RemoveAll(this);

	if (UInventoryWidget* ParentWidget = GetFirstOccurenceOuter<UInventoryWidget>())
	{
		ParentWidget->OnEquipped.RemoveAll(this);
		ParentWidget->OnUnequipped.RemoveAll(this);
	}
}

void UOwnedCountWidgetEntry::RetrieveEntitlementWithForceRequest(const bool bForceRequest)
{
	SetVisibility(ESlateVisibility::Collapsed);

	W_Parent = GetFirstOccurenceOuter<UStoreItemListEntry>();
	if (W_Parent)
	{
		// Get item.
		const UStoreItemDataObject* ItemData = W_Parent->GetItemData();
		if (!ItemData)
		{
			return;
		}

		EntitlementsSubsystem->GetOrQueryUserItemEntitlement(
			GetOwningPlayer(),
			ItemData->GetStoreItemId(),
			FOnGetOrQueryUserItemEntitlementComplete::CreateUObject(this, &ThisClass::ShowOwnedCount),
			bForceRequest);
	}
}

void UOwnedCountWidgetEntry::ShowOwnedCount(const FOnlineError& Error, const UStoreItemDataObject* Entitlement)
{
	if (!Error.bSucceeded || !Entitlement)
	{
		return;
	}

	// Set owned count.
	FText Text;
	if (Entitlement->GetCount() > 0)
	{
		Text = Entitlement->GetIsConsumable() ?
			FText::FromString(FString::Printf(TEXT("%d x"), Entitlement->GetCount())) : TEXT_OWNED;
	}
	Tb_OwnedCount->SetText(Text);

	// Show the owned count if not empty.
	SetVisibility(Tb_OwnedCount->GetText().IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (UInventoryWidget* ParentWidget = GetFirstOccurenceOuter<UInventoryWidget>())
	{
		CheckItemEquipped();
		ParentWidget->OnEquipped.AddUObject(this, &ThisClass::CheckItemEquipped);
		ParentWidget->OnUnequipped.AddUObject(this, &ThisClass::CheckItemEquipped);
	}
}

void UOwnedCountWidgetEntry::CheckItemEquipped()
{
	// Only if currently on inventory menu
	if (!GetFirstOccurenceOuter<UInventoryWidget>())
	{
		return;
	}

	const UStoreItemListEntry* ParentWidget = GetFirstOccurenceOuter<UStoreItemListEntry>();
	if (!ParentWidget)
	{
		return;
	}

	const UStoreItemDataObject* Item = ParentWidget->GetItemData();
	if (!Item)
	{
		return;
	}

	// Highlight the entitlement item if it is equipped and inside the inventory menu
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	bool bEquipped = false;
	if (!Item->GetSkuMap().IsEmpty() && Item->GetSkuMap().Find(EItemSkuPlatform::AccelByte))
	{
		bEquipped = GameInstance->IsItemEquippedBySku(
			GetOwningPlayer()->GetLocalPlayer()->GetControllerId(),
			EItemSkuPlatform::AccelByte,
			Item->GetSkuMap()[EItemSkuPlatform::AccelByte]);
	}
	W_Parent->Execute_ToggleHighlight(W_Parent, bEquipped);
}
