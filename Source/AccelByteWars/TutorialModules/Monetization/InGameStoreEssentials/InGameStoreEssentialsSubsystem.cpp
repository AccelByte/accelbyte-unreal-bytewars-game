// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "Monetization/NativePlatformPurchase/NativePlatformPurchaseModels.h"

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-Initialize
// @@@MULTISNIP StoreInterface {"selectedLines": ["1-2", "5-9"]}
void UInGameStoreEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	StoreInterface = Subsystem->GetStoreV2Interface();
	ensure(StoreInterface);
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetOrQueryOffersByCategory
void UInGameStoreEssentialsSubsystem::GetOrQueryOffersByCategory(
	const APlayerController* PlayerController,
	const FString& Category,
	FOnGetOrQueryOffersByCategory OnComplete,
	bool bForceRefresh)
{
	// Check overall cache.
	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);

	// If empty or forced to refresh, call query.
	if (Offers.IsEmpty() || bForceRefresh)
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete]()
			{
				OnComplete.Execute(TArray<UStoreItemDataObject*>());
			}));
			return;
		}

		OffersByCategoryDelegates.Add(Category, OnComplete);
		QueryOffers(LocalUserNetId);
	}
	// Else, call get.
	else
	{
		const TArray<UStoreItemDataObject*> StoreItemDataObjects = GetOffersByCategory(Category);
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete, StoreItemDataObjects]()
		{
			OnComplete.Execute(StoreItemDataObjects);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetOrQueryOfferById
void UInGameStoreEssentialsSubsystem::GetOrQueryOfferById(
	const APlayerController* PlayerController,
	const FUniqueOfferId& OfferId,
	FOnGetOrQueryOfferById OnComplete)
{
	// Check overall cache.
	TArray<FOnlineStoreOfferRef> Offers;
	StoreInterface->GetOffers(Offers);

	// If empty, call query.
	if (Offers.IsEmpty())
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete]()
			{
				OnComplete.Execute(nullptr);
			}));
			return;
		}

		OfferByIdDelegates.Add(OfferId, OnComplete);
		QueryOffers(LocalUserNetId);
	}
	// Else, call get.
	else
	{
		UStoreItemDataObject* StoreItemDataObject = GetOfferById(OfferId);
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete, StoreItemDataObject]()
		{
			OnComplete.Execute(StoreItemDataObject);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetOrQueryCategoriesByRootPath
void UInGameStoreEssentialsSubsystem::GetOrQueryCategoriesByRootPath(
	const APlayerController* PlayerController,
	const FString& RootPath,
	FOnGetOrQueryCategories OnComplete,
	bool bForceRefresh)
{
	// Check overall cache.
	TArray<FOnlineStoreCategory> Categories;
	StoreInterface->GetCategories(Categories);

	// If empty or forced to refresh, query.
	if (Categories.IsEmpty() || bForceRefresh)
	{
		const FUniqueNetIdPtr LocalUserNetId = GetUniqueNetIdFromPlayerController(PlayerController);
		if (!LocalUserNetId.IsValid())
		{
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete]()
			{
				OnComplete.Execute(TArray<FOnlineStoreCategory>());
			}));
			return;
		}

		CategoriesByRootPathDelegates.Add(RootPath, OnComplete);
		QueryCategories(LocalUserNetId);
	}
	// Else, execute immediately.
	else
	{
		const TArray<FOnlineStoreCategory> StoreCategories = GetCategories(RootPath);
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [OnComplete, StoreCategories]()
		{
			OnComplete.Execute(StoreCategories);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetOffersByCategory
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
		StoreItems.Add(FInGameStoreEssentialsUtils::ConvertStoreData(Offer.Get()));
	}

	return StoreItems;
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetOfferById
UStoreItemDataObject* UInGameStoreEssentialsSubsystem::GetOfferById(const FUniqueOfferId& OfferId) const
{
	UStoreItemDataObject* StoreItem = nullptr;
	if (const TSharedPtr<FOnlineStoreOffer> Offer = StoreInterface->GetOffer(OfferId); Offer.IsValid())
	{
		StoreItem = FInGameStoreEssentialsUtils::ConvertStoreData(*Offer.Get());
	}
	return StoreItem;
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-QueryOffers
void UInGameStoreEssentialsSubsystem::QueryOffers(const FUniqueNetIdPtr UserId)
{
	// Abort if the query process is already running.
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
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-OnQueryOffersComplete
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

		// Avoid modifying while it still being used.
		OffersByCategoryDelegateToBeDeleted.Add(&Delegate.Key);
	}

	// Delete delegates.
	for (const FString* Key : OffersByCategoryDelegateToBeDeleted)
	{
		OffersByCategoryDelegates.Remove(*Key);
	}

	TArray<const FUniqueOfferId*> OfferByIdDelegateToBeDeleted;
	for (TTuple<const FUniqueOfferId, FOnGetOrQueryOfferById>& Delegate : OfferByIdDelegates)
	{
		Delegate.Value.Execute(GetOfferById(Delegate.Key));

		// Avoid modifying while it still being used.
		OfferByIdDelegateToBeDeleted.Add(&Delegate.Key);
	}

	// Delete delegates.
	for (const FUniqueOfferId* Key : OfferByIdDelegateToBeDeleted)
	{
		OfferByIdDelegates.Remove(*Key);
	}
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-GetCategories
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
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-QueryCategories
void UInGameStoreEssentialsSubsystem::QueryCategories(const FUniqueNetIdPtr UserId)
{
	// Abort if the query process is already running.
	if (bIsQueryCategoriesRunning)
	{
		return;
	}

	// If bound, meaning NativePlatformPurchaseSubsystem is active
	if (FNativePlatformPurchaseUtils::OnQueryItemMapping.IsBound())
	{
		FNativePlatformPurchaseUtils::OnQueryItemMapping.Execute(UserId, FOnQueryItemMappingCompleted::CreateWeakLambda(this, [this, UserId](const FNativeItemPricingMap& Pricing)
		{
			StoreInterface->QueryCategories(*UserId.Get(), FOnQueryOnlineStoreCategoriesComplete::CreateUObject(this, &ThisClass::OnQueryCategoriesComplete));
		}));
	}
	// If not bound, meaning the NativePlatformPurchaseSubsystem is not active
	else
	{
		StoreInterface->QueryCategories(*UserId.Get(), FOnQueryOnlineStoreCategoriesComplete::CreateUObject(this, &ThisClass::OnQueryCategoriesComplete));
	}
	bIsQueryCategoriesRunning = true;
}
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.cpp-OnQueryCategoriesComplete
void UInGameStoreEssentialsSubsystem::OnQueryCategoriesComplete(bool bWasSuccessful, const FString& Error)
{
	bIsQueryCategoriesRunning = false;

	TArray<const FString*> KeysToDelete;
	for (TTuple<const FString, FOnGetOrQueryCategories>& Delegate : CategoriesByRootPathDelegates)
	{
		Delegate.Value.Execute(GetCategories(Delegate.Key));

		// Avoid modifying while it still being used.
		KeysToDelete.Add(&Delegate.Key);
	}

	// Delete delegates.
	for (const FString* Key : KeysToDelete)
	{
		CategoriesByRootPathDelegates.Remove(*Key);
	}
}
// @@@SNIPEND

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

#pragma endregion 
