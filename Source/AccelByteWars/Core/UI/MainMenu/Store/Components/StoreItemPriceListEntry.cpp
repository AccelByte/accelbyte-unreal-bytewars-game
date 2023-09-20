// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPriceListEntry.h"

#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Components/TextBlock.h"

void UStoreItemPriceListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// Reset UI
	const FText EmptyText = FText();
	Tb_RegularPrice->SetText(EmptyText);
	Tb_FinalPrice->SetText(EmptyText);
	Tb_CurrencySymbol_RegularPrice->SetText(EmptyText);
	Tb_CurrencySymbol_FinalPrice->SetText(EmptyText);
	Tb_CurrencySymbol_RegularPrice->SetVisibility(ESlateVisibility::Visible);
	Tb_RegularPrice->SetVisibility(ESlateVisibility::Visible);
	Tb_CurrencySymbol_FinalPrice->SetVisibility(ESlateVisibility::Visible);

	// Get data
	const UStoreItemPriceDataObject* PriceDataObject = static_cast<UStoreItemPriceDataObject*>(ListItemObject);
	if (!PriceDataObject)
	{
		return;
	}

	// Set UI
	// Set Currency Symbol
	const FText CurrencySymbol = FText::FromString(PriceDataObject->CurrencyCode);
	Tb_CurrencySymbol_RegularPrice->SetText(CurrencySymbol);
	Tb_CurrencySymbol_FinalPrice->SetText(CurrencySymbol);

	// Set prices
	const bool bIsRegularPriceFree = PriceDataObject->RegularPrice == 0;
	const FText RegularPrice = bIsRegularPriceFree ?
		TEXT_PRICE_FREE :
		FText::FromString(FString::Printf(TEXT("%lld"), PriceDataObject->RegularPrice));
	Tb_RegularPrice->SetText(RegularPrice);
	if (bIsRegularPriceFree)
	{
		Tb_CurrencySymbol_RegularPrice->SetVisibility(ESlateVisibility::Collapsed);
	}

	const bool bIsFinalPriceFree = PriceDataObject->FinalPrice == 0;
	const FText FinalPrice = bIsFinalPriceFree ?
		TEXT_PRICE_FREE :
		FText::FromString(FString::Printf(TEXT("%lld"), PriceDataObject->FinalPrice));
	Tb_FinalPrice->SetText(FinalPrice);
	if (bIsFinalPriceFree)
	{
		Tb_CurrencySymbol_FinalPrice->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Hide regular price if not discounted
	if (!PriceDataObject->IsDiscounted())
	{
		Tb_CurrencySymbol_RegularPrice->SetVisibility(ESlateVisibility::Collapsed);
		Tb_RegularPrice->SetVisibility(ESlateVisibility::Collapsed);
	}
}
