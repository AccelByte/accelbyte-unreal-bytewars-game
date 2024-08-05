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

private:
	struct FUserItemEntitlementRequest
	{
		const APlayerController* PlayerController;
		FOnGetOrQueryUserItemEntitlementComplete OnComplete;
	};
	struct FConsumeEntitlementRequest
	{
		const APlayerController* PlayerController;
		FOnConsumeUserEntitlementComplete OnComplete;
	};

	TMultiMap<const APlayerController*, FOnGetOrQueryUserEntitlementsComplete> UserEntitlementsParams;
	TMultiMap<const FUniqueOfferId /*OfferId*/, FUserItemEntitlementRequest> UserItemEntitlementParams;
	TMultiMap<const FString /*InGameItemId*/, FConsumeEntitlementRequest> ConsumeEntitlementParams;

	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	uint8 QueryProcess = 0;
	UPROPERTY() TArray<UStoreItemDataObject*> StoreOffers;
	FUniqueNetIdPtr QueryResultUserId;
	FOnlineError QueryResultError;
	void QueryUserEntitlement(const APlayerController* PlayerController);
	void OnQueryEntitlementComplete(
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Namespace,
		const FString& ErrorMessage);
	void OnQueryStoreOfferComplete(TArray<UStoreItemDataObject*> Offers);
	void CompleteQuery();

	void OnConsumeEntitlementComplete(
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const TSharedPtr<FOnlineEntitlement>& Entitlement,
		const FOnlineError& Error);

	UStoreItemDataObject* GetItemEntitlement(const FUniqueNetIdPtr UserId, const FUniqueOfferId OfferId) const;

	void UpdatePlayerEquipmentQuantity(const APlayerController* PlayerController, const TArray<FString>& ItemIds);
	void OnPowerUpActivated(const APlayerController* PlayerController, const FString& ItemId);

#pragma region "Utilities"
	TArray<UStoreItemDataObject*> EntitlementsToDataObjects(TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const;
	UStoreItemDataObject* EntitlementToDataObject(TSharedRef<FOnlineEntitlement> Entitlement) const;
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
#pragma endregion
};
