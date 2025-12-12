// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "OwnedCountWidgetEntry.h"

#include "Components/TextBlock.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget_Starter.h"
#include "Monetization/EntitlementsEssentials/UI/EquipmentInventoryWidget.h"

#define PARENT_WIDGET_CLASS UEquipmentInventoryWidget

// @@@SNIPSTART OwnedCountWidgetEntry.cpp-NativeOnActivated
// @@@MULTISNIP Subsystem {"selectedLines": ["1-2", "5-6", "18"]}
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
	UItemPurchaseWidget_Starter::OnPurchaseCompleteMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		RetrieveEntitlementWithForceRequest(true);
	});
}
// @@@SNIPEND

// @@@SNIPSTART OwnedCountWidgetEntry.cpp-NativeOnDeactivated
void UOwnedCountWidgetEntry::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.RemoveAll(this);
	UItemPurchaseWidget_Starter::OnPurchaseCompleteMulticastDelegate.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART OwnedCountWidgetEntry.cpp-RetrieveEntitlementWithForceRequest
// @@@MULTISNIP Setup {"selectedLines": ["1-13", "21-22"]}
void UOwnedCountWidgetEntry::RetrieveEntitlementWithForceRequest(const bool bForceRequest)
{
	SetVisibility(ESlateVisibility::Collapsed);

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	W_Parent = GetFirstOccurenceOuter<UStoreItemListEntry>();
	if (W_Parent && LocalPlayer)
	{
		// Get item.
		const UStoreItemDataObject* ItemData = W_Parent->GetItemData();
		if (!ItemData)
		{
			return;
		}

		EntitlementsSubsystem->GetOrQueryUserItemEntitlement(
			LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
			ItemData->GetStoreItemId(),
			FOnGetOrQueryUserItemEntitlementComplete::CreateUObject(this, &ThisClass::ShowOwnedCount),
			bForceRequest);
	}
}
// @@@SNIPEND

// @@@SNIPSTART OwnedCountWidgetEntry.cpp-ShowOwnedCount
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
}
// @@@SNIPEND

#undef PARENT_WIDGET_CLASS
