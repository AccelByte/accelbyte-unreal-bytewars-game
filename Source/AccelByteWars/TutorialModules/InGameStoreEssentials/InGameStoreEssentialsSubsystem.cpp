// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"

void UInGameStoreEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	StoreInterface = Subsystem->GetStoreV2Interface();
	ensure(StoreInterface);
}

TArray<UStoreItemDataObject*> UInGameStoreEssentialsSubsystem::GetOffersByCategory(const FString Category) const
{
	TArray<FOnlineStoreOfferRef> FilteredOffers;
	TArray<UStoreItemDataObject*> StoreItems;

	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);
	for (const FOnlineStoreOfferRef& Offer : Offers)
	{
		if (Offer->DynamicFields.Find(TEXT("Category"))->Find(Category) == 0)
		{
			FilteredOffers.Add(Offer);
		}
	}

	for (const TSharedRef<FOnlineStoreOffer>& Offer : FilteredOffers)
	{
		StoreItems.Add(ConvertStoreData(Offer.Get()));
	}

	return StoreItems;
}

UStoreItemDataObject* UInGameStoreEssentialsSubsystem::GetOfferById(const FUniqueOfferId& OfferId) const
{
	UStoreItemDataObject* StoreItem = nullptr;
	if (const TSharedPtr<FOnlineStoreOffer> Offer = StoreInterface->GetOffer(OfferId); Offer.IsValid())
	{
		StoreItem = ConvertStoreData(*Offer.Get());
	}
	return StoreItem;
}

void UInGameStoreEssentialsSubsystem::QueryOffers(const APlayerController* PlayerController)
{
	const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);

	// safety
	if (!LocalUserNetId.IsValid())
	{
		return;
	}

	StoreInterface->QueryOffersByFilter(
		*LocalUserNetId.Get(),
		FOnlineStoreFilter(),
		FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &ThisClass::OnQueryOffersComplete));
	bIsQueryRunning = true;
}

void UInGameStoreEssentialsSubsystem::OnQueryOffersComplete(
	bool bWasSuccessful,
	const TArray<FUniqueOfferId>& OfferIds,
	const FString& Error)
{
	bIsQueryRunning = false;

	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);

	OnQueryOfferByCategoryCompleteDelegate.Broadcast(bWasSuccessful, Error);
}

FUniqueNetIdPtr UInGameStoreEssentialsSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const
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

UStoreItemDataObject* UInGameStoreEssentialsSubsystem::ConvertStoreData(const FOnlineStoreOffer& Offer) const
{
	UStoreItemDataObject* StoreItem = NewObject<UStoreItemDataObject>();
	UItemDataObject* Item = NewObject<UItemDataObject>();
	Item->Title = Offer.Title;
	Item->Id = Offer.OfferId;

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

	UStoreItemPriceDataObject* PriceData = NewObject<UStoreItemPriceDataObject>();
	PriceData->RegularPrice = Offer.RegularPrice;
	PriceData->FinalPrice = Offer.NumericPrice;
	PriceData->CurrencyCode = Offer.CurrencyCode;

	StoreItem->Prices.Add(PriceData);
	StoreItem->ItemData = Item;

	return StoreItem;
}
