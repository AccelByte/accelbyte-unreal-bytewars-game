// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPriceListEntry.h"

#include "Components/Image.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Components/TextBlock.h"

#if WITH_EDITOR
void UStoreItemPriceListEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		I_CurrencySymbol->SetBrush(DebugCurrencyType == ECurrencyType::COIN ? Brush_Coin : Brush_Gem);
	}
}
#endif

void UStoreItemPriceListEntry::Setup(const UStoreItemPriceDataObject* DataObject, const int32 Multiplier) const
{
	if (!DataObject)
	{
		return;
	}

	// Set currency symbol
	const ESlateVisibility NewVisibility = (DataObject->GetCurrencyType() != ECurrencyType::NONE) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (NewVisibility == ESlateVisibility::Visible)
	{
		I_CurrencySymbol->SetBrush(DataObject->GetCurrencyType() == ECurrencyType::COIN ? Brush_Coin : Brush_Gem);
	}
	I_CurrencySymbol->SetVisibility(NewVisibility);

	// Set prices
	const bool bIsRegularPriceFree = DataObject->GetRegularPrice() == 0;
	const FText RegularPrice = bIsRegularPriceFree ?
		TEXT_PRICE_FREE :
		FText::FromString(FString::Printf(TEXT("%lld"), DataObject->GetRegularPrice() * Multiplier));
	Tb_RegularPrice->SetText(RegularPrice);

	const bool bIsFinalPriceFree = DataObject->GetFinalPrice() == 0;
	const FText FinalPrice = bIsFinalPriceFree ?
		TEXT_PRICE_FREE :
		FText::FromString(FString::Printf(TEXT("%lld"), DataObject->GetFinalPrice() * Multiplier));
	Tb_FinalPrice->SetText(FinalPrice);

	// Hide regular price if not discounted
	if (!DataObject->IsDiscounted())
	{
		Tb_RegularPrice->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UStoreItemPriceListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// Get data
	const UStoreItemPriceDataObject* PriceDataObject = static_cast<UStoreItemPriceDataObject*>(ListItemObject);
	if (!PriceDataObject)
	{
		return;
	}

	Setup(PriceDataObject);
}

void UStoreItemPriceListEntry::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();

	ResetUI();
}

void UStoreItemPriceListEntry::ResetUI() const
{
	const FText EmptyText = FText();
	Tb_RegularPrice->SetText(EmptyText);
	Tb_FinalPrice->SetText(EmptyText);
	Tb_RegularPrice->SetVisibility(ESlateVisibility::Hidden);
	Tb_FinalPrice->SetVisibility(ESlateVisibility::Visible);
	I_CurrencySymbol->SetVisibility(ESlateVisibility::Visible);
	I_CurrencySymbol->SetBrush(Brush_Coin);
}
