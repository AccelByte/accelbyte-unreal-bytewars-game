// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "OwnedCountWidgetEntry_Starter.h"

#include "Components/TextBlock.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem_Starter.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

#include "Monetization/EntitlementsEssentials/UI/InventoryWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget_Starter.h"

#define PARENT_WIDGET_CLASS UInventoryWidget_Starter

void UOwnedCountWidgetEntry_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem_Starter>();
	ensure(EntitlementsSubsystem);

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion
}

void UOwnedCountWidgetEntry_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 
}

void UOwnedCountWidgetEntry_Starter::CheckItemEquipped()
{
	// Only if currently on inventory menu
	if (!GetFirstOccurenceOuter<PARENT_WIDGET_CLASS>())
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

#pragma region "Tutorial"
// Put your code here.
#pragma endregion 

#undef PARENT_WIDGET_CLASS
