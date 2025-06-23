// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InGameStoreEssentialsModel.h"
#include "OnlineSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "InGameStoreEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UInGameStoreEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

// @@@SNIPSTART InGameStoreEssentialsSubsystem.h-public
// @@@MULTISNIP GetOrQueryCategoriesByRootPath {"selectedLines": ["1", "11-15"]}
// @@@MULTISNIP QueryStoreItems {"selectedLines": ["1-10"]}
public:
	void GetOrQueryOffersByCategory(
		const APlayerController* PlayerController,
		const FString& Category,
		FOnGetOrQueryOffersByCategory OnComplete,
		bool bForceRefresh = false);
	void GetOrQueryOfferById(
		const APlayerController* PlayerController,
		const FUniqueOfferId& OfferId,
		FOnGetOrQueryOfferById OnComplete);
	void GetOrQueryCategoriesByRootPath(
		const APlayerController* PlayerController,
		const FString& RootPath,
		FOnGetOrQueryCategories OnComplete,
		bool bForceRefresh = false);
// @@@SNIPEND

// @@@SNIPSTART InGameStoreEssentialsSubsystem.h-private
// @@@MULTISNIP StoreInterface {"selectedLines": ["1-2"]}
// @@@MULTISNIP GetUniqueNetIdFromPlayerController {"selectedLines": ["1", "22"]}
// @@@MULTISNIP ConvertStoreData {"selectedLines": ["1", "23-24"]}
// @@@MULTISNIP CategoriesDelegate {"selectedLines": ["1", "6"]}
// @@@MULTISNIP CategoriesIndication {"selectedLines": ["1", "17"]}
// @@@MULTISNIP CategoriesFunction {"selectedLines": ["1", "15", "18-19"]}
// @@@MULTISNIP StoreItemsDelegate {"selectedLines": ["1", "4-5"]}
// @@@MULTISNIP StoreItemsIndication {"selectedLines": ["1", "11"]}
// @@@MULTISNIP StoreItemsFunctions {"selectedLines": ["1", "8-9", "12-13"]}
private:
	IOnlineStoreV2Ptr StoreInterface;

	TMultiMap<const FString /*Category*/, FOnGetOrQueryOffersByCategory> OffersByCategoryDelegates;
	TMultiMap<const FUniqueOfferId /*OfferId*/, FOnGetOrQueryOfferById> OfferByIdDelegates;
	TMultiMap<const FString /*CategoryPath*/, FOnGetOrQueryCategories> CategoriesByRootPathDelegates;

	TArray<UStoreItemDataObject*> GetOffersByCategory(const FString Category) const;
	UStoreItemDataObject* GetOfferById(const FUniqueOfferId& OfferId) const;

	bool bIsQueryOfferRunning = false;
	void QueryOffers(const FUniqueNetIdPtr UserId);
	void OnQueryOffersComplete(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error);

	TArray<FOnlineStoreCategory> GetCategories(const FString& RootPath) const;

	bool bIsQueryCategoriesRunning = false;
	void QueryCategories(const FUniqueNetIdPtr UserId);
	void OnQueryCategoriesComplete(bool bWasSuccessful, const FString& Error);

#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	UStoreItemDataObject* ConvertStoreData(
		const FOnlineStoreOffer& Offer) const;
#pragma endregion
// @@@SNIPEND
};
