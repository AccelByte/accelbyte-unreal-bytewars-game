// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem_Starter.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"

void UInGameStoreEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	StoreInterface = Subsystem->GetStoreV2Interface();
	ensure(StoreInterface);
}

#pragma region "Tutorial"
// put your code here
#pragma endregion 

#pragma region "Utilities"
FUniqueNetIdPtr UInGameStoreEssentialsSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const
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

UStoreItemDataObject* UInGameStoreEssentialsSubsystem_Starter::ConvertStoreData(
	const FOnlineStoreOffer& Offer,
	const FString* ParentCategory) const
{
	UStoreItemDataObject* StoreItem = NewObject<UStoreItemDataObject>();
	UItemDataObject* Item = NewObject<UItemDataObject>();
	Item->Title = Offer.Title;
	Item->Id = Offer.OfferId;
	Item->Category = FText::FromString(*Offer.DynamicFields.Find(TEXT("Category")));

	if (const FString* Sku = Offer.DynamicFields.Find(TEXT("Sku")))
	{
		Item->Sku = *Sku;
	}
	if (const FString* Consumable = Offer.DynamicFields.Find(TEXT("IsConsumable")))
	{
		Item->bConsumable = Consumable->Equals("true");
	}
	if (const FString* ItemCategory = Offer.DynamicFields.Find(TEXT("Category")))
	{
		StoreItem->Category = *ItemCategory;
	}
	if (const FString* ItemType = Offer.DynamicFields.Find(TEXT("ItemType")))
	{
		StoreItem->ItemType = *ItemType;
	}
	if (const FString* IconUrl = Offer.DynamicFields.Find(TEXT("IconUrl")))
	{
		Item->IconUrl = *IconUrl;
	}

	const FOnlineStoreOfferAccelByte* OfferAccelByte = (FOnlineStoreOfferAccelByte*) &Offer;
	if (!OfferAccelByte)
	{
		return StoreItem;
	}

	for (const FAccelByteModelsItemRegionDataItem& RegionData : OfferAccelByte->RegionData)
	{
		UStoreItemPriceDataObject* PriceData = NewObject<UStoreItemPriceDataObject>();
		PriceData->RegularPrice = RegionData.Price;
		PriceData->FinalPrice = RegionData.DiscountedPrice;
		PriceData->CurrencyType = RegionData.CurrencyCode == TEXT("BC") ? ECurrencyType::COIN : ECurrencyType::GEM;

		StoreItem->Prices.Add(PriceData);
	}

	StoreItem->ItemData = Item;

	return StoreItem;
}
#pragma endregion 
