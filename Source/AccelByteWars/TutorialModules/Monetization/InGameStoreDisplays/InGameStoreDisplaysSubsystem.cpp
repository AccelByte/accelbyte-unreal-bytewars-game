// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreDisplaysSubsystem.h"

#include "OnlineSubsystemUtils.h"

void UInGameStoreDisplaysSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
	const IOnlineStoreV2Ptr StorePtr = Subsystem->GetStoreV2Interface();
	ensure(StorePtr);
	StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(StorePtr);
	ensure(StoreInterface);
}

void UInGameStoreDisplaysSubsystem::QueryOrGetDisplays(
	const APlayerController* PlayerController,
	const FOnQueryOrGetDisplaysComplete& OnComplete,
	const bool bForceRefresh)
{
	const FUniqueNetIdPtr UserId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!UserId.IsValid())
	{
		return;
	}

	// check cache
	if (TArray<TSharedRef<FAccelByteModelsViewInfo>> Displays = GetDisplays(); !Displays.IsEmpty() && !bForceRefresh)
	{
		OnComplete.ExecuteIfBound(Displays, FOnlineError::Success());
		return;
	}

	// cache empty, trigger query
	QueryOrGetDisplaysOnCompleteDelegates.Add(OnComplete);
	QueryStoreFront(*UserId.Get());
}

void UInGameStoreDisplaysSubsystem::QueryOrGetSectionsForDisplay(
	const APlayerController* PlayerController,
	const FString& DisplayId,
	const FOnQueryOrGetSectionsInDisplaComplete& OnComplete,
	const bool bForceRefresh)
{
	const FUniqueNetIdPtr UserId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!UserId.IsValid())
	{
		return;
	}

	// check cache
	if (TArray<TSharedRef<FAccelByteModelsSectionInfo>> Sections = GetSectionsForDisplay(*UserId.Get(), DisplayId);
		!Sections.IsEmpty() && !bForceRefresh)
	{
		OnComplete.ExecuteIfBound(Sections, FOnlineError::Success());
		return;
	}

	// cache empty, trigger query
	QueryOrGetSectionsOnCompleteDelegates.Add({UserId, DisplayId, OnComplete});
	QueryStoreFront(*UserId.Get());
}

void UInGameStoreDisplaysSubsystem::QueryOrGetOffersInSection(
	const APlayerController* PlayerController,
	const FString& SectionId,
	const FOnQueryOrGetOffersInSectionComplete& OnComplete,
	const bool bForceRefresh)
{
	const FUniqueNetIdPtr UserId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!UserId.IsValid())
	{
		return;
	}

	// check cache
	if (TArray<UStoreItemDataObject*> Offers = GetOffersForSection(*UserId.Get(), SectionId);
		!Offers.IsEmpty() && !bForceRefresh)
	{
		OnComplete.ExecuteIfBound(Offers, FOnlineError::Success());
		return;
	}

	// cache empty, trigger query
	QueryOrGetOffersOnCompleteDelegates.Add({UserId, SectionId, OnComplete});
	if (bIsQueryOffersRunning)
	{
		return;
	}
	bIsQueryOffersRunning = true;
	StoreInterface->QueryOffersByFilter(
		*UserId.Get(),
		FOnlineStoreFilter(),
		FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &ThisClass::OnQueryOffersComplete));
}

TArray<TSharedRef<FAccelByteModelsViewInfo>> UInGameStoreDisplaysSubsystem::GetDisplays() const
{
	TArray<TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>> Displays;
	StoreInterface->GetDisplays(Displays);
	return Displays;
}

TArray<TSharedRef<FAccelByteModelsSectionInfo>> UInGameStoreDisplaysSubsystem::GetSectionsForDisplay(
	const FUniqueNetId& UserId,
	const FString& DisplayId) const
{
	TArray<TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>> Sections;
	StoreInterface->GetSectionsForDisplay(UserId, DisplayId, Sections);
	return Sections;
}

TArray<UStoreItemDataObject*> UInGameStoreDisplaysSubsystem::GetOffersForSection(
	const FUniqueNetId& UserId,
	const FString& SectionId) const
{
	TArray<FOnlineStoreOfferAccelByteRef> Offers;
	StoreInterface->GetOffersForSection(UserId, SectionId, Offers);

	TArray<UStoreItemDataObject*> OfferObjects;
	for (const FOnlineStoreOfferAccelByteRef& Offer : Offers)
	{
		OfferObjects.Add(ConvertStoreData(Offer));
	}
	return OfferObjects;
}

void UInGameStoreDisplaysSubsystem::QueryStoreFront(const FUniqueNetId& UserId)
{
	if (bIsQueryStoreFrontRunning)
	{
		return;
	}
	bIsQueryStoreFrontRunning = true;

	FString StoreId;
	FString ViewId;
	FString Region;
	constexpr EAccelBytePlatformMapping PlatformMapping = EAccelBytePlatformMapping::NONE;
	StoreInterface->QueryStorefront(
		UserId,
		StoreId,
		ViewId,
		Region,
		PlatformMapping,
		FOnQueryStorefrontComplete::CreateUObject(this, &ThisClass::OnQueryStoreFrontComplete));
}

void UInGameStoreDisplaysSubsystem::OnQueryStoreFrontComplete(
	bool bWasSuccessful,
	const TArray<FString>& ViewIds,
	const TArray<FString>& SectionIds,
	const TArray<FUniqueOfferId>& OfferIds,
	const TArray<FString>& ItemMappingIds,
	const FString& Error)
{
	bIsQueryStoreFrontRunning = false;

	FOnlineError OnlineError;
	OnlineError.bSucceeded = bWasSuccessful;
	OnlineError.ErrorMessage = FText::FromString(Error);

	for (const FOnQueryOrGetDisplaysComplete& Param : QueryOrGetDisplaysOnCompleteDelegates)
	{
		TArray<TSharedRef<FAccelByteModelsViewInfo>> Displays = GetDisplays();
		Param.ExecuteIfBound(Displays, OnlineError);
	}
	QueryOrGetDisplaysOnCompleteDelegates.Empty();

	for (FQueryOrGetSectionsParam& Param : QueryOrGetSectionsOnCompleteDelegates)
	{
		TArray<TSharedRef<FAccelByteModelsSectionInfo>> Sections = GetSectionsForDisplay(*Param.UserId.Get(), Param.DisplayId);
		Param.OnComplete.ExecuteIfBound(Sections, OnlineError);
	}
	QueryOrGetSectionsOnCompleteDelegates.Empty();
}

void UInGameStoreDisplaysSubsystem::OnQueryOffersComplete(
	bool bWasSuccessful,
	const TArray<FUniqueOfferId>& OfferIds,
	const FString& Error)
{
	bIsQueryOffersRunning = false;

	FOnlineError OnlineError;
	OnlineError.bSucceeded = bWasSuccessful;
	OnlineError.ErrorMessage = FText::FromString(Error);

	for (FQueryOrGetOffersParam& Param : QueryOrGetOffersOnCompleteDelegates)
	{
		TArray<UStoreItemDataObject*> Offers = GetOffersForSection(*Param.UserId.Get(), Param.SectionId);
		Param.OnComplete.ExecuteIfBound(Offers, OnlineError);
	}
	QueryOrGetOffersOnCompleteDelegates.Empty();
}

#pragma region "Utilities"
FUniqueNetIdPtr UInGameStoreDisplaysSubsystem::GetUniqueNetIdFromPlayerController(
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

UStoreItemDataObject* UInGameStoreDisplaysSubsystem::ConvertStoreData(const FOnlineStoreOfferAccelByteRef Offer) const
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
