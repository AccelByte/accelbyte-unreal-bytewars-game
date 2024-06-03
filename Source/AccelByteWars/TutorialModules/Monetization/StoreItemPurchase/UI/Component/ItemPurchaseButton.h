// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "ItemPurchaseButton.generated.h"

class UStoreItemPriceListEntry;

UCLASS(Abstract)
class ACCELBYTEWARS_API UItemPurchaseButton : public UAccelByteWarsButtonBase
{
	GENERATED_BODY()

public:
	void SetPrice(const UStoreItemPriceDataObject* PriceData, const int32 PriceMultiplier = 1) const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UStoreItemPriceListEntry* W_Price;
};
