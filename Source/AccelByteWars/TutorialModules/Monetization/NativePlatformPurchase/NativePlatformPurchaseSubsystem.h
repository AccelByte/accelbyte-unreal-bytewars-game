// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "NativePlatformPurchaseModels.h"
#if PLATFORM_STEAM
#include "steam/steam_api.h"
#endif

#include "NativePlatformPurchaseSubsystem.generated.h"
ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogNativePlatformPurchaseSubsystem, Log, All);

#define UE_LOG_NATIVE_PLATFORM_PURCHASE(Verbosity, Format, ...) \
{ \
	UE_LOG(LogNativePlatformPurchaseSubsystem, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

UCLASS()
class ACCELBYTEWARS_API UNativePlatformPurchaseSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	void QueryItemMapping(
		FUniqueNetIdPtr UserId, FOnQueryItemMappingCompleted OnQueryCompleted);

	TArray<TSharedRef<FAccelByteModelsItemMapping>> GetItemMapping();
	const FNativeItemPricingMap& GetItemPrices() const { return ItemPriceMap; }

	void CheckoutItem(const APlayerController* OwningPlayer, const TWeakObjectPtr<UStoreItemDataObject> StoreItemData);

	bool OpenPlatformStore(
		const APlayerController* OwningPlayer,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const;
	FOnRequestCompleted OnSyncPurchaseCompleteDelegates;

	EPurchaseState PurchaseSyncState {NotStarted};

	FString GetNativePlatformName() const;
	bool IsNativePlatformSupported() const;
	bool IsItemSupportedByNativePlatform(const FString& Item) const;
	
	FOnQueryItemMappingCompleted OnQueryItemMappingCompleted;

private:
	FOnlineStoreV2AccelBytePtr StoreInterface;
	FOnlineEntitlementsAccelBytePtr EntitlementInterface;
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	bool bIsQueryItemMappingRunning = false;
	void OnQueryItemMappingComplete(
		bool bWasSuccessful,
		const TArray<FString>& ViewIds,
		const TArray<FString>& SectionIds,
		const TArray<FUniqueOfferId>& OfferIds,
		const TArray<FString>& ItemMappingIds,
		const FString& Error,
		const FUniqueNetIdPtr UserId);
	
	FNativeItemPricingMap ItemPriceMap;
	
	void OnSyncPurchaseCompleted(bool bWasSuccess, const FString& ErrorMessage);
	FTimerHandle SyncPurchaseTimerHandle;

#pragma region "Steam"
	bool OpenSteamStore(const APlayerController* OwningPlayer,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping,
		const IOnlineSubsystem* PlatformSubsystem) const;

	bool TryGetSteamStoreUrl(
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping,
		FString& OutSteamStoreUrl) const;

	FAccelByteModelsEntitlementSyncBase GetItemToSync(
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping)const;

#if PLATFORM_STEAM
	void QuerySteamItemDefinition();
	const TMap<FString, FNativeItemPrice>& GetSteamItemPricing(const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings);
	CCallResult<UNativePlatformPurchaseSubsystem, SteamInventoryRequestPricesResult_t> InventoryRequestPricesResult{};
	void OnSteamRequestPricesCompleted(SteamInventoryRequestPricesResult_t* ResultCallback, bool bIoFailure);
	STEAM_CALLBACK(UNativePlatformPurchaseSubsystem, HandleSteamPurchaseOverlay, GameOverlayActivated_t);
	TMap<int32, uint64> SteamItemPrices;
#endif
#pragma endregion
	
#pragma region "Google Play"
#if PLATFORM_ANDROID
	void CheckoutItemWithGooglePlay(
		const APlayerController* OwningPlayer,
		const FPurchaseCheckoutRequest CheckoutRequest,
		const bool bIsConsumeable);

	const TMap<FString, FNativeItemPrice>& GetGooglePlayItemPricing(
		const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings);

	void QueryGooglePlayItems(
		const FUniqueNetIdPtr UserId,
		const TArray<FUniqueOfferId>& ItemIds,
		const FOnQueryOnlineStoreOffersComplete& OnComplete);

	bool ParseGooglePlayReceiptToSyncRequest(
		const TSharedRef<FPurchaseReceipt>& Receipt, 
		FAccelByteModelsPlatformSyncMobileGoogle& OutRequest);

	IOnlineIdentityPtr GooglePlayIdentityInterface;
	IOnlinePurchasePtr GooglePlayPurchaseInterface;
	IOnlineStoreV2Ptr GooglePlayStoreInterface;
#endif
#pragma endregion

#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	FString GetItemsMappingId(const FString ProductId, const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const;

	const TMap<EAccelBytePlatformType, EAccelBytePlatformMapping> SupportedNativePlatform =
	{
		{ EAccelBytePlatformType::Steam, EAccelBytePlatformMapping::STEAM },
		{ EAccelBytePlatformType::Google, EAccelBytePlatformMapping::GOOGLE },
		{ EAccelBytePlatformType::GooglePlayGames, EAccelBytePlatformMapping::GOOGLE },
	};

	const TArray<EAccelByteItemType> SupportedNativePlatformItem = { EAccelByteItemType::COINS };
#pragma endregion 
};