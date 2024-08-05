// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"

void UInGameStoreEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	StoreInterface = Subsystem->GetStoreV2Interface();
	ensure(StoreInterface);
}

void UInGameStoreEssentialsSubsystem::GetOrQueryOffersByCategory(
	const APlayerController* PlayerController,
	const FString& Category,
	FOnGetOrQueryOffersByCategory OnComplete,
	bool bForceRefresh)
{
	// check overall cache
	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);

	// If empty or forced to refresh, call query
	if (Offers.IsEmpty() || bForceRefresh)
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			OnComplete.Execute(TArray<UStoreItemDataObject*>());
			return;
		}

		OffersByCategoryDelegates.Add(Category, OnComplete);
		QueryOffers(LocalUserNetId);
	}
	// else, call get
	else
	{
		OnComplete.Execute(GetOffersByCategory(Category));
	}
}

void UInGameStoreEssentialsSubsystem::GetOrQueryOfferById(
	const APlayerController* PlayerController,
	const FUniqueOfferId& OfferId,
	FOnGetOrQueryOfferById OnComplete)
{
	// check overall cache
	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);

	// If empty, call query
	if (Offers.IsEmpty())
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			OnComplete.Execute(nullptr);
			return;
		}

		OfferByIdDelegates.Add(OfferId, OnComplete);
		QueryOffers(LocalUserNetId);
	}
	// else, call get
	else
	{
		OnComplete.Execute(GetOfferById(OfferId));
	}
}

void UInGameStoreEssentialsSubsystem::GetOrQueryCategoriesByRootPath(
	const APlayerController* PlayerController,
	const FString& RootPath,
	FOnGetOrQueryCategories OnComplete,
	bool bForceRefresh)
{
	// check overall cache
	TArray<FOnlineStoreCategory> Categories;
	StoreInterface->GetCategories(Categories);

	// if empty or forced to refresh, query
	if (Categories.IsEmpty() || bForceRefresh)
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			OnComplete.Execute(TArray<FOnlineStoreCategory>());
			return;
		}

		CategoriesByRootPathDelegates.Add(RootPath, OnComplete);
		QueryCategories(LocalUserNetId);
	}
	// else, execute immediately
	else
	{
		OnComplete.Execute(GetCategories(RootPath));
	}
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

void UInGameStoreEssentialsSubsystem::QueryOffers(const FUniqueNetIdPtr UserId)
{
	// safety
	if (bIsQueryOfferRunning)
	{
		return;
	}

	StoreInterface->QueryOffersByFilter(
		*UserId.Get(),
		FOnlineStoreFilter(),
		FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &ThisClass::OnQueryOffersComplete));
	bIsQueryOfferRunning = true;
}

void UInGameStoreEssentialsSubsystem::OnQueryOffersComplete(
	bool bWasSuccessful,
	const TArray<FUniqueOfferId>& OfferIds,
	const FString& Error)
{
	bIsQueryOfferRunning = false;

	TArray<const FString*> OffersByCategoryDelegateToBeDeleted;
	for (TTuple<const FString /*Category*/, FOnGetOrQueryOffersByCategory>& Delegate : OffersByCategoryDelegates)
	{
		Delegate.Value.Execute(GetOffersByCategory(Delegate.Key));

		// avoid modifying while it still being used
		OffersByCategoryDelegateToBeDeleted.Add(&Delegate.Key);
	}

	// delete delegates
	for (const FString* Key : OffersByCategoryDelegateToBeDeleted)
	{
		OffersByCategoryDelegates.Remove(*Key);
	}

	TArray<const FUniqueOfferId*> OfferByIdDelegateToBeDeleted;
	for (TTuple<const FUniqueOfferId, FOnGetOrQueryOfferById>& Delegate : OfferByIdDelegates)
	{
		Delegate.Value.Execute(GetOfferById(Delegate.Key));

		// avoid modifying while it still being used
		OfferByIdDelegateToBeDeleted.Add(&Delegate.Key);
	}

	// delete delegates
	for (const FUniqueOfferId* Key : OfferByIdDelegateToBeDeleted)
	{
		OfferByIdDelegates.Remove(*Key);
	}
}

TArray<FOnlineStoreCategory> UInGameStoreEssentialsSubsystem::GetCategories(const FString& RootPath) const
{
	TArray<FOnlineStoreCategory> Categories;
	StoreInterface->GetCategories(Categories);

	TArray<FOnlineStoreCategory> ChildCategories;
	for (FOnlineStoreCategory& Category : Categories)
	{
		if (Category.Id.Find(RootPath) == 0)
		{
			ChildCategories.Add(Category);
		}
	}

	return ChildCategories;
}

void UInGameStoreEssentialsSubsystem::QueryCategories(const FUniqueNetIdPtr UserId)
{
	// safety
	if (bIsQueryCategoriesRunning)
	{
		return;
	}

	StoreInterface->QueryCategories(
		*UserId.Get(),
		FOnQueryOnlineStoreCategoriesComplete::CreateUObject(this, &ThisClass::OnQueryCategoriesComplete));
	bIsQueryCategoriesRunning = true;
}

void UInGameStoreEssentialsSubsystem::OnQueryCategoriesComplete(bool bWasSuccessful, const FString& Error)
{
	bIsQueryCategoriesRunning = false;

	TArray<const FString*> KeysToDelete;
	for (TTuple<const FString, FOnGetOrQueryCategories>& Delegate : CategoriesByRootPathDelegates)
	{
		Delegate.Value.Execute(GetCategories(Delegate.Key));

		// avoid modifying while it still being used
		KeysToDelete.Add(&Delegate.Key);
	}

	// delete delegates
	for (const FString* Key : KeysToDelete)
	{
		CategoriesByRootPathDelegates.Remove(*Key);
	}
}

#pragma region "Utilities"
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
#pragma endregion 
