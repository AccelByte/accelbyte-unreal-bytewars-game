// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InGameStoreDisplaysModel.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Play/MatchSessionDS/MatchSessionDSOnlineSession_Starter.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "InGameStoreDisplaysSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UInGameStoreDisplaysSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

// @@@SNIPSTART InGameStoreDisplaysSubsystem.h-public
public:
	/**
	 * @brief Retrieve store displays info from endpoint / cache if exist
	 * @param PlayerController User's player controller to use
	 * @param OnComplete Executed on response received
	 * @param bForceRefresh if true, force call endpoint
	 */
	void QueryOrGetDisplays(
		const APlayerController* PlayerController,
		const FOnQueryOrGetDisplaysComplete& OnComplete,
		const bool bForceRefresh = false);

	/**
	 * @brief Retrieve store display's sections info from endpoint / cache if exist
	 * @param PlayerController User's player controller to use
	 * @param DisplayId Display Id to retrieve from
	 * @param OnComplete Executed on response received
	 * @param bForceRefresh if true, force call endpoint
	 */
	void QueryOrGetSectionsForDisplay(
		const APlayerController* PlayerController,
		const FString& DisplayId,
		const FOnQueryOrGetSectionsInDisplayComplete& OnComplete,
		const bool bForceRefresh = false);

	/**
	 * @brief Retrieve store section's offers info from endpoint / cache if exist
	 * @param PlayerController User's player controller to use
	 * @param SectionId Section Id to retrieve from
	 * @param OnComplete Executed on response received
	 * @param bForceRefresh if true, force call endpoint
	 */
	void QueryOrGetOffersInSection(
		const APlayerController* PlayerController,
		const FString& SectionId,
		const FOnQueryOrGetOffersInSectionComplete& OnComplete,
		const bool bForceRefresh = false);
// @@@SNIPEND

// @@@SNIPSTART InGameStoreDisplaysSubsystem.h-private
// @@@MULTISNIP StoreInterface {"selectedLines": ["1-2"]}
// @@@MULTISNIP Params {"selectedLines": ["1", "4-6"]}
// @@@MULTISNIP Cache {"selectedLines": ["1", "8-12"]}
// @@@MULTISNIP Query {"selectedLines": ["1", "14-28"]}
// @@@MULTISNIP GetUniqueNetIdFromPlayerController {"selectedLines": ["1", "31"]}
// @@@MULTISNIP ConvertStoreData {"selectedLines": ["1", "32"]}
private:
	FOnlineStoreV2AccelBytePtr StoreInterface;

	TArray<FOnQueryOrGetDisplaysComplete> QueryOrGetDisplaysOnCompleteDelegates;
	TArray<FQueryOrGetSectionsParam> QueryOrGetSectionsOnCompleteDelegates;
	TArray<FQueryOrGetOffersParam> QueryOrGetOffersOnCompleteDelegates;

	TArray<TSharedRef<FAccelByteModelsViewInfo>> GetDisplays() const;
	TArray<TSharedRef<FAccelByteModelsSectionInfo>> GetSectionsForDisplay(
		const FUniqueNetId& UserId,
		const FString& DisplayId) const;
	TArray<UStoreItemDataObject*> GetOffersForSection(const FUniqueNetId& UserId, const FString& SectionId) const;

	bool bIsQueryStoreFrontRunning = false;
	void QueryStoreFront(const FUniqueNetId& UserId);
	void OnQueryStoreFrontComplete(
		bool bWasSuccessful,
		const TArray<FString>& ViewIds,
		const TArray<FString>& SectionIds,
		const TArray<FUniqueOfferId>& OfferIds,
		const TArray<FString>& ItemMappingIds,
		const FString& Error);

	bool bIsQueryOffersRunning = false;
	void OnQueryOffersComplete(
		bool bWasSuccessful,
		const TArray<FUniqueOfferId>& OfferIds,
		const FString& Error);

#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	UStoreItemDataObject* ConvertStoreData(const FOnlineStoreOfferAccelByteRef Offer) const;
#pragma endregion
// @@@SNIPEND
};
