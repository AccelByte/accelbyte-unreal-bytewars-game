// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "EntitlementsEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"
#include "EntitlementsEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UEntitlementsEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

// @@@SNIPSTART EntitlementsEssentialsSubsystem.h-public
// @@@MULTISNIP Query {"selectedLines": ["1-10"]}
// @@@MULTISNIP Consume {"selectedLines": ["1", "11-20"]}
public:
	void GetOrQueryUserEntitlements(
		const APlayerController* PlayerController,
		const FOnGetOrQueryUserEntitlementsComplete& OnComplete,
		const bool bForceRequest = false);
	void GetOrQueryUserItemEntitlement(
		const APlayerController* PlayerController,
		const FUniqueOfferId& StoreItemId,
		const FOnGetOrQueryUserItemEntitlementComplete& OnComplete,
		const bool bForceRequest = false);
	void ConsumeItemEntitlementByInGameId(
		const APlayerController* PlayerController,
		const FString& InGameItemId,
		const int32 UseCount = 1,
		const FOnConsumeUserEntitlementComplete& OnComplete = FOnConsumeUserEntitlementComplete());
	void ConsumeEntitlementByEntitlementId(
		const APlayerController* PlayerController,
		const FString& EntitlementId,
		const int32 UseCount = 1,
		const FOnConsumeUserEntitlementComplete& OnComplete = FOnConsumeUserEntitlementComplete());
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.h-private
// @@@MULTISNIP Interface {"selectedLines": ["1-2"]}
// @@@MULTISNIP StoreOffers {"selectedLines": ["1", "4-5"]}
// @@@MULTISNIP ObjectConversion {"selectedLines": ["1", "35-36"]}
// @@@MULTISNIP GetLocalPlayerUniqueNetId {"selectedLines": ["1", "37"]}
// @@@MULTISNIP DelegatesQuery {"selectedLines": ["1", "7-8"]}
// @@@MULTISNIP DelegatesConsume {"selectedLines": ["1", "9"]}
// @@@MULTISNIP Query {"selectedLines": ["1", "11-23"]}
// @@@MULTISNIP OnConsumeEntitlementComplete {"selectedLines": ["1", "25-29"]}
// @@@MULTISNIP OnPowerUpActivated {"selectedLines": ["1", "31"]}
private:
	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	UPROPERTY()
	TArray<UStoreItemDataObject*> StoreOffers;

	TMultiMap<const APlayerController*, FOnGetOrQueryUserEntitlementsComplete> UserEntitlementsParams;
	TMultiMap<const FUniqueOfferId /*OfferId*/, FUserItemEntitlementRequest> UserItemEntitlementParams;
	TMultiMap<const FString /*InGameItemId*/, FConsumeEntitlementRequest> ConsumeEntitlementParams;

	uint8 QueryProcess = 0;
	FUniqueNetIdPtr QueryResultUserId;
	FOnlineError QueryResultError;

	void QueryUserEntitlement(const APlayerController* PlayerController);
	void OnQueryEntitlementComplete(
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Namespace,
		const FString& ErrorMessage);
	void OnQueryStoreOfferComplete(TArray<UStoreItemDataObject*> Offers);
	UStoreItemDataObject* GetItemEntitlement(const FUniqueNetIdPtr UserId, const FUniqueOfferId OfferId) const;
	void CompleteQuery();

	void OnConsumeEntitlementComplete(
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const TSharedPtr<FOnlineEntitlement>& Entitlement,
		const FOnlineError& Error);

	void OnPowerUpActivated(const APlayerController* PlayerController, const FString& ItemId);
	void UpdatePlayerEquipmentQuantity(const APlayerController* PlayerController, const TArray<FString>& ItemIds);

#pragma region "Utilities"
	TArray<UStoreItemDataObject*> EntitlementsToDataObjects(TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const;
	UStoreItemDataObject* EntitlementToDataObject(TSharedRef<FOnlineEntitlement> Entitlement) const;
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion
// @@@SNIPEND
};
