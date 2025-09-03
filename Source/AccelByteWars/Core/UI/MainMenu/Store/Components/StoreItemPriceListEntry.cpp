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
		switch (DebugCurrencyType) {
		case ECurrencyType::COIN:
			I_CurrencySymbol->SetBrush(Brush_Coin);
			I_CurrencySymbol->SetVisibility(ESlateVisibility::Visible);
			break;
		case ECurrencyType::GEM:
			I_CurrencySymbol->SetBrush(Brush_Gem);
			I_CurrencySymbol->SetVisibility(ESlateVisibility::Visible);
			break;
		case ECurrencyType::NATIVE:
		default:
			I_CurrencySymbol->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
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
	ESlateVisibility NewVisibility = ESlateVisibility::Visible;
	switch (DataObject->GetCurrencyType()) 
	{
	case ECurrencyType::COIN:
		I_CurrencySymbol->SetBrush(Brush_Coin);
		break;
	case ECurrencyType::GEM:
		I_CurrencySymbol->SetBrush(Brush_Gem);
		break;
	default:
		NewVisibility = ESlateVisibility::Collapsed;
		break;
	}
	I_CurrencySymbol->SetVisibility(NewVisibility);

	// Set prices
	const bool bIsRegularPriceFree = DataObject->GetRegularPrice() <= 0;
	const FText RegularPrice = bIsRegularPriceFree ?
		TEXT_PRICE_FREE :
		FText::FromString(FString::Printf(TEXT("%lld"), DataObject->GetRegularPrice() * Multiplier));
	Tb_RegularPrice->SetText(RegularPrice);

	const bool bIsFinalPriceFree = DataObject->GetFinalPrice() <= 0;
	const FText FinalPrice = 
		bIsFinalPriceFree ? TEXT_PRICE_FREE :
		DataObject->GetCurrencyType() == ECurrencyType::NATIVE ?
		FORMAT_NATIVE_CURRENCY(DataObject->GetFinalPrice(), DataObject->GetNativeCurrencyCode()) :
		FText::FromString(FString::Printf(TEXT("%lld"), DataObject->GetFinalPrice() * Multiplier));
	Tb_FinalPrice->SetText(FinalPrice);

	// Hide regular price if not discounted
	Tb_RegularPrice->SetVisibility(
		DataObject->IsDiscounted() ? 
		ESlateVisibility::Visible : ESlateVisibility::Collapsed);
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
	Tb_RegularPrice->SetVisibility(ESlateVisibility::Collapsed);
	Tb_FinalPrice->SetVisibility(ESlateVisibility::Visible);
	I_CurrencySymbol->SetVisibility(ESlateVisibility::Visible);
	I_CurrencySymbol->SetBrush(Brush_Coin);
}
