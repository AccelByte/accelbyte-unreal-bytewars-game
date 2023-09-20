// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemListEntry.h"

#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"

void UStoreItemListEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	Lv_Prices->SetVisibility(bShowPrices ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UStoreItemListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (UStoreItemDataObject* StoreItemDataObject = Cast<UStoreItemDataObject>(ListItemObject))
	{
		Setup(StoreItemDataObject);
	}
	else if (UItemDataObject* ItemDataObject = Cast<UItemDataObject>(ListItemObject))
	{
		Setup(ItemDataObject);
	}
}

void UStoreItemListEntry::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();

	if (W_EntitlementOuter->HasAnyChildren())
	{
		if (UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(W_EntitlementOuter->GetChildAt(0)))
		{
			Widget->DeactivateWidget();
		}
	}
}

void UStoreItemListEntry::Setup(UItemDataObject* Object)
{
	ItemData = Object;

	// Reset UI
	Tb_Name->SetText(FText());
	W_Image->LoadImage("");

	// Set UI
	Tb_Name->SetText(ItemData->Title);
	W_Image->LoadImage(ItemData->IconUrl);

	if (W_EntitlementOuter->HasAnyChildren())
	{
		if (UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(W_EntitlementOuter->GetChildAt(0)))
		{
			Widget->ActivateWidget();
		}
	}
}

void UStoreItemListEntry::Setup(UStoreItemDataObject* Object)
{
	Setup(Object->ItemData);

	// Reset UI
	Lv_Prices->ClearListItems();

	// Set UI
	if (bShowPrices)
	{
		Lv_Prices->SetListItems(Object->Prices);
	}
}
