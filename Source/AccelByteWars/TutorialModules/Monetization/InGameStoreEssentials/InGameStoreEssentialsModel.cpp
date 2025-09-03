// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "InGameStoreEssentialsModel.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "Monetization/NativePlatformPurchase/NativePlatformPurchaseModels.h"

UStoreItemDataObject* FInGameStoreEssentialsUtils::ConvertStoreData(const FOnlineStoreOffer& Offer)
{
	TMap<EItemSkuPlatform, FString> SkuMap;
	if (const FString* SkuPtr = Offer.DynamicFields.Find(TEXT("Sku")))
	{
		SkuMap.Add(EItemSkuPlatform::AccelByte, *SkuPtr);
	}
	bool bConsumable = false;
	if (const FString* ConsumablePtr = Offer.DynamicFields.Find(TEXT("IsConsumable")))
	{
		bConsumable = ConsumablePtr->Equals("true");
	}
	FText Category;
	if (const FString* CategoryPtr = Offer.DynamicFields.Find(TEXT("Category")))
	{
		Category = FText::FromString(*CategoryPtr);
	}
	FString ItemType;
	if (const FString* ItemTypePtr = Offer.DynamicFields.Find(TEXT("ItemType")))
	{
		ItemType = *ItemTypePtr;
	}
	FString IconUrl;
	if (const FString* IconUrlPtr = Offer.DynamicFields.Find(TEXT("IconUrl")))
	{
		IconUrl = *IconUrlPtr;
	}

	TArray<UStoreItemPriceDataObject*> Prices;
	
	if (FNativePlatformPurchaseUtils::OnGetItemPrices.IsBound()) 
	{
		TMap<FString, FNativeItemPrice> NativeItemPrices = FNativePlatformPurchaseUtils::OnGetItemPrices.Execute();
		if (NativeItemPrices.Num() > 0)
		{
			for (const TPair<FString, FNativeItemPrice>& ItemPrice : NativeItemPrices)
			{
				const FString ABSku = 
					SkuMap.Contains(EItemSkuPlatform::AccelByte) ? 
					SkuMap[EItemSkuPlatform::AccelByte] : TEXT("");

				// Check if the native item pricing is available by comparing either the ItemId or the AccelByte SKU.
				if (ItemPrice.Key.Equals(Offer.OfferId) ||
					ItemPrice.Key.Equals(ABSku))
				{
					SkuMap.Add(EItemSkuPlatform::Native, ItemPrice.Value.Id);

					UStoreItemPriceDataObject* PriceData = NewObject<UStoreItemPriceDataObject>();
					PriceData->Setup(
						ECurrencyType::NATIVE,
						ItemPrice.Value.Price,
						ItemPrice.Value.Price,
						FNativePlatformPurchaseUtils::RegionalCurrencyCode
					);
					Prices.Add(PriceData);
				}
			}
		}
	}
	
	if(Prices.Num() == 0)
	{
		if (const FOnlineStoreOfferAccelByte* OfferAccelByte = (FOnlineStoreOfferAccelByte*) &Offer)
		{
			for (const FAccelByteModelsItemRegionDataItem& RegionData : OfferAccelByte->RegionData)
			{
				UStoreItemPriceDataObject* PriceData = NewObject<UStoreItemPriceDataObject>();
				PriceData->Setup(
					FPreConfigCurrency::GetTypeFromCode(RegionData.CurrencyCode),
					RegionData.Price,
					RegionData.DiscountedPrice);

				Prices.Add(PriceData);
			}
		}
	}

	UStoreItemDataObject* Item = NewObject<UStoreItemDataObject>();
	const FString EmptyEntitlementId;
	constexpr int32 Count = 1;
	Item->Setup(
		Offer.Title,
		Category,
		ItemType,
		Offer.OfferId,
		EmptyEntitlementId,
		IconUrl,
		SkuMap,
		Prices,
		Count,
		bConsumable);
	return Item;
}