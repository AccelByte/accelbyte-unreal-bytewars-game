// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreDisplaysSubsystem_Starter.h"

#include "OnlineSubsystemUtils.h"

void UInGameStoreDisplaysSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	const IOnlineStoreV2Ptr StorePtr = Subsystem->GetStoreV2Interface();
	ensure(StorePtr);
	StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(StorePtr);
	ensure(StoreInterface);
}

#pragma region "Tutorial"
// Put your code here
#pragma endregion

#pragma region "Utilities"
FUniqueNetIdPtr UInGameStoreDisplaysSubsystem_Starter::GetUniqueNetIdFromPlayerController(
	const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	const int LocalUserNum = LocalPlayer->GetControllerId();

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	return Subsystem->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
}

UStoreItemDataObject* UInGameStoreDisplaysSubsystem_Starter::ConvertStoreData(const FOnlineStoreOfferAccelByteRef Offer) const
{
	TMap<EItemSkuPlatform, FString> SkuMap;
	if (const FString* SkuPtr = Offer->DynamicFields.Find(TEXT("Sku")))
	{
		SkuMap.Add(EItemSkuPlatform::AccelByte, *SkuPtr);
	}
	bool bConsumable = false;
	if (const FString* ConsumablePtr = Offer->DynamicFields.Find(TEXT("IsConsumable")))
	{
		bConsumable = ConsumablePtr->Equals("true");
	}
	FText Category;
	if (const FString* CategoryPtr = Offer->DynamicFields.Find(TEXT("Category")))
	{
		Category = FText::FromString(*CategoryPtr);
	}
	FString ItemType;
	if (const FString* ItemTypePtr = Offer->DynamicFields.Find(TEXT("ItemType")))
	{
		ItemType = *ItemTypePtr;
	}
	FString IconUrl;
	if (const FString* IconUrlPtr = Offer->DynamicFields.Find(TEXT("IconUrl")))
	{
		IconUrl = *IconUrlPtr;
	}

	TArray<UStoreItemPriceDataObject*> Prices;
	if (FOnlineStoreOfferAccelByte* OfferAccelByte = static_cast<FOnlineStoreOfferAccelByte*>(&Offer.Get()))
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

	UStoreItemDataObject* Item = NewObject<UStoreItemDataObject>();
	const FString EmptyEntitlementId;
	constexpr int32 Count = 1;
	Item->Setup(
		Offer->Title,
		Category,
		ItemType,
		Offer->OfferId,
		EmptyEntitlementId,
		IconUrl,
		SkuMap,
		Prices,
		Count,
		bConsumable);
	return Item;
}
#pragma endregion 
