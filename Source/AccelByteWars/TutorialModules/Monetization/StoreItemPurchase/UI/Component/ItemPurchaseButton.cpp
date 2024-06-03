// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ItemPurchaseButton.h"

#include "Core/UI/MainMenu/Store/Components/StoreItemPriceListEntry.h"

void UItemPurchaseButton::SetPrice(const UStoreItemPriceDataObject* PriceData, const int32 PriceMultiplier) const
{
	W_Price->Setup(PriceData, PriceMultiplier);
}
