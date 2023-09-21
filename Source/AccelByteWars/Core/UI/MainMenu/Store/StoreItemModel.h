// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "StoreItemModel.generated.h"

#define TEXT_PRICE_FREE NSLOCTEXT("AccelByteWars", "free", "Free")
#define TEXT_PURCHASE NSLOCTEXT("AccelByteWars", "purchase-with", "Purchase with")

UCLASS()
class UItemDataObject : public UObject
{
	GENERATED_BODY()

public:
	FText Title;
	FString Id;
	FString Sku;
	FString IconUrl;

	int32 Count;
	bool bConsumable;
};

UCLASS()
class UStoreItemPriceDataObject : public UObject
{
	GENERATED_BODY()

public:
	FString CurrencyCode;
	int64 RegularPrice;
	int64 FinalPrice;

	bool IsDiscounted() const { return RegularPrice != FinalPrice; }
};

UCLASS()
class UStoreItemDataObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UItemDataObject* ItemData;

	UPROPERTY()
	TArray<UStoreItemPriceDataObject*> Prices;

	FString Category;
	FString ItemType;
};
