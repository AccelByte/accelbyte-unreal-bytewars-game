// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "EntitlementsEssentialsLog.h"
#include "EntitlementsEssentialsModel.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
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
// @@@MULTISNIP GetUserEquipments {"selectedLines": ["1", "22-25"]}
// @@@MULTISNIP SetUserEquipments {"selectedLines": ["1", "26-30"]}
public:
	void GetOrQueryUserEntitlements(
		const FUniqueNetIdPtr UserId,
		const FOnGetOrQueryUserEntitlementsComplete& OnComplete,
		const bool bForceRequest = false);
	void GetOrQueryUserItemEntitlement(
		const FUniqueNetIdPtr UserId,
		const FUniqueOfferId& StoreItemId,
		const FOnGetOrQueryUserItemEntitlementComplete& OnComplete,
		const bool bForceRequest = false);
	void ConsumeItemEntitlementByInGameId(
		const FUniqueNetIdPtr UserId,
		const FString& InGameItemId,
		const int32 UseCount = 1,
		const FOnConsumeUserEntitlementComplete& OnComplete = FOnConsumeUserEntitlementComplete());
	void ConsumeEntitlementByEntitlementId(
		const FUniqueNetIdPtr UserId,
		const FString& EntitlementId,
		const int32 UseCount = 1,
		const FOnConsumeUserEntitlementComplete& OnComplete = FOnConsumeUserEntitlementComplete());

	void GetUserEquipments(
		const int32 LocalUserNum,
		const FUniqueNetIdPtr UserId, 
		const FOnUpdateUserEquipmentsComplete& OnComplete = FOnUpdateUserEquipmentsComplete());
	void SetUserEquipments(
		const int32 LocalUserNum,
		const FUniqueNetIdPtr UserId, 
		const FPlayerEquipments& Equipments, 
		const FOnUpdateUserEquipmentsComplete& OnComplete = FOnUpdateUserEquipmentsComplete());
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.h-private
// @@@MULTISNIP Interfaces {"selectedLines": ["1-4"]}
// @@@MULTISNIP StoreOffers {"selectedLines": ["1", "6-7"]}
// @@@MULTISNIP ObjectConversion {"selectedLines": ["1", "61-62"]}
// @@@MULTISNIP DelegatesQuery {"selectedLines": ["1", "12-13"]}
// @@@MULTISNIP DelegatesConsume {"selectedLines": ["1", "14"]}
// @@@MULTISNIP Query {"selectedLines": ["1", "16-28"]}
// @@@MULTISNIP OnConsumeEntitlementComplete {"selectedLines": ["1", "30-34"]}
// @@@MULTISNIP UpdateUserEquipments {"selectedLines": ["1", "30-34"]}
// @@@MULTISNIP OnGetSetUserEquipmentsComplete {"selectedLines": ["1", "42-58"]}
private:
	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	UPROPERTY()
	TArray<UStoreItemDataObject*> StoreOffers;

	UPROPERTY()
	FPlayerEquipments CurrentEquipments;

	TMultiMap<const FUniqueNetIdRef, FOnGetOrQueryUserEntitlementsComplete> UserEntitlementsParams;
	TMultiMap<const FUniqueOfferId /*OfferId*/, FUserItemEntitlementRequest> UserItemEntitlementParams;
	TMultiMap<const FString /*InGameItemId*/, FConsumeEntitlementRequest> ConsumeEntitlementParams;

	uint8 QueryProcess = 0;
	FUniqueNetIdPtr QueryResultUserId;
	FOnlineError QueryResultError;

	void QueryUserEntitlement(const FUniqueNetIdPtr UserId);
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

	void UpdateUserEquipments(
		const int32 LocalUserNum,
		const FUniqueNetIdPtr UserId,
		const FPlayerEquipments& Equipments,
		const FOnUpdateUserEquipmentsComplete& OnComplete);

	void OnGetUserEquipmentsComplete(
		const int32 LocalUserNum,
		const FOnlineError& Result,
		const FString& Key,
		const FAccelByteModelsUserRecord& Record,
		const FUniqueNetIdRef UserId,
		const FOnUpdateUserEquipmentsComplete OnComplete);

	void OnSetUserEquipmentsComplete(
		int32 LocalUserNum, 
		const FOnlineError& Result, 
		const FString& Key,
		const FUniqueNetIdRef UserId,
		const FPlayerEquipments Equipments,
		const FOnUpdateUserEquipmentsComplete OnComplete);

	FDelegateHandle OnGetUserEquipmentsCompleteDelegateHandle, OnSetUserEquipmentsCompleteDelegateHandle;

#pragma region "Utilities"
	TArray<UStoreItemDataObject*> EntitlementsToDataObjects(TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const;
	UStoreItemDataObject* EntitlementToDataObject(TSharedRef<FOnlineEntitlement> Entitlement) const;
#pragma endregion
// @@@SNIPEND
};
