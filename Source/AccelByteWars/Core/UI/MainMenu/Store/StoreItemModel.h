// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "StoreItemModel.generated.h"

#define TEXT_PRICE_FREE NSLOCTEXT("AccelByteWars", "free", "Free")
#define TEXT_PURCHASE NSLOCTEXT("AccelByteWars", "purchase-with", "Purchase with")

UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	COIN = 0,
	GEM
};

UCLASS()
class UItemDataObject : public UObject
{
	GENERATED_BODY()

public:
	FText Title;
	FText Category;
	FString Id;
	FString Sku;
	FString IconUrl;

	int32 Count;
	bool bConsumable;
};

UCLASS(BlueprintType)
class UStoreItemPriceDataObject : public UObject
{
	GENERATED_BODY()

	UStoreItemPriceDataObject(){}
	UStoreItemPriceDataObject(ECurrencyType Type, int64 Price) : CurrencyType(Type), RegularPrice(Price), FinalPrice(Price) {}
	UStoreItemPriceDataObject(ECurrencyType Type, int64 Price, int64 DiscountedPrice) : CurrencyType(Type), RegularPrice(DiscountedPrice), FinalPrice(Price) {}

public:
	UPROPERTY(EditAnywhere)
	ECurrencyType CurrencyType;

	UPROPERTY(EditAnywhere)
	int64 RegularPrice;

	UPROPERTY(EditAnywhere)
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
