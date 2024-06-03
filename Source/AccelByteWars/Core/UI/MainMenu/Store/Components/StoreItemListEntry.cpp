// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemListEntry.h"

#include "Components/Border.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
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
		SetOwned(false);
	}
	else if (UItemDataObject* ItemDataObject = Cast<UItemDataObject>(ListItemObject))
	{
		Setup(ItemDataObject);
	}
}

void UStoreItemListEntry::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();
	ResetUI();
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

	// Only show end of branch category
	FString Category = ItemData->Category.ToString();
	Category = Category.RightChop(Category.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);
	Tb_Category->SetText(FText::FromString(Category));

	if (W_EntitlementOuter->HasAnyChildren())
	{
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
}

void UStoreItemListEntry::Setup(UStoreItemDataObject* Object)
{
	Setup(Object->ItemData);

	// Reset UI
	Lv_Prices->ClearListItems();

	// Set UI
	if (bShowPrices)
	{
		// Show only 1 price if one of them is free
		bool bIsFree = false;
		for (const UStoreItemPriceDataObject* Data : Object->Prices)
		{
			if (Data->RegularPrice == 0)
			{
				bIsFree = true;
			}
		}

		if (bIsFree)
		{
			TArray<UStoreItemPriceDataObject*> Prices;
			if (Object->Prices.Num() < 1)
			{
				return;
			}

			Prices.Add(Object->Prices[0]);
			Lv_Prices->SetListItems(Prices);
		}
		else
		{
			Lv_Prices->SetListItems(Object->Prices);
		}
	}
}

void UStoreItemListEntry::SetOwned(const bool bOwned) const
{
	if (bOwned)
	{
		Ws_Price->SetActiveWidget(Tb_Owned);
	}
	else
	{
		Ws_Price->SetActiveWidget(Lv_Prices);
	}

	B_Root->SetContentColorAndOpacity(bOwned ? OwnedColor : NormalColor);
}

void UStoreItemListEntry::ResetUI() const
{
	if (W_EntitlementOuter->HasAnyChildren())
	{
		if (UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(W_EntitlementOuter->GetChildAt(0)))
		{
			Widget->DeactivateWidget();
		}
	}
	Lv_Prices->ClearListItems();
}
