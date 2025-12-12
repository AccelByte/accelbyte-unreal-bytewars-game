// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemListEntry.h"

#include "Components/Border.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"

void UStoreItemListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UStoreItemDataObject* StoreItemDataObject = Cast<UStoreItemDataObject>(ListItemObject))
	{
		Setup(StoreItemDataObject);
	}
}

void UStoreItemListEntry::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();
	ResetUI();
}

void UStoreItemListEntry::Setup(const UStoreItemDataObject* Object)
{
	ItemData = Object;

	// Reset UI
	Tb_Name->SetText(FText());
	Lv_Prices->ClearListItems();

	// Set UI
	Tb_Name->SetText(ItemData->GetTitle());
	W_Image->LoadImage(ItemData->GetIconUrl());
	Lv_Prices->SetVisibility(ItemData->GetIsShowPrices() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ItemData->GetIsShowPrices())
	{
		Lv_Prices->SetListItems(Object->GetPrices());
	}

	// Only show end of branch category
	FString Category = ItemData->GetCategory().ToString();
	Category = Category.RightChop(Category.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);
	Tb_Category->SetText(FText::FromString(Category));

	// Determine whether to show entitlement item count or not.
	if (ItemData->GetIsShowItemCount() && W_EntitlementOuter->HasAnyChildren())
	{
		W_EntitlementOuter->SetVisibility(ESlateVisibility::Visible);
		if (UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(W_EntitlementOuter->GetChildAt(0)))
		{
			// Activate or reactivate widget.
			if (Widget->IsActivated()) 
			{
				Widget->DeactivateWidget();
			}
			Widget->ActivateWidget();
		}
	}
	else
	{
		W_EntitlementOuter->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UStoreItemListEntry::ResetUI()
{
	if (W_EntitlementOuter->HasAnyChildren())
	{
		if (UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(W_EntitlementOuter->GetChildAt(0)))
		{
			Widget->DeactivateWidget();
		}
	}
	Lv_Prices->ClearListItems();

	Execute_ToggleHighlight(this, false);
}
