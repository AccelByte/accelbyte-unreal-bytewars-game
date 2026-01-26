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
	virtual void Deinitialize() override;

public:
	void SyncAllEntitlements(
		const FUniqueNetIdPtr UserId, 
		const FOnRequestCompleted& OnComplete = FOnRequestCompleted(),
		const FAccelByteModelsEntitlementSyncBase& EntitlementSyncBase = FAccelByteModelsEntitlementSyncBase());
	
	void CheckoutItem(
		const FUniqueNetIdPtr UserId,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData);

	void QueryItemMapping(
		const FUniqueNetIdPtr UserId,
		const FOnQueryItemMappingCompleted OnQueryCompleted);

	bool IsNativePlatformSupported(const FUniqueNetIdPtr UserId) const;

	TArray<TSharedRef<FAccelByteModelsItemMapping>> GetItemMapping();
	const FNativeItemPricingMap& GetItemPrices() const { return ItemPriceMap; }

	FOnQueryItemMappingCompleted OnQueryItemMappingCompleted;
	FOnRequestCompleted OnSyncPurchaseCompleteDelegates;

	EPurchaseState PurchaseSyncState{ NotStarted };

private:
	void CheckoutWithNativePlatform(
		const FUniqueNetIdPtr UserId,
		const FPurchaseCheckoutRequest CheckoutRequest,
		const bool bIsConsumeable);

	void QueryItemsWithNativePlatform(
		const FUniqueNetIdPtr UserId,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings);

	void OnQueryItemMappingComplete(
		bool bWasSuccessful,
		const TArray<FString>& ViewIds,
		const TArray<FString>& SectionIds,
		const TArray<FUniqueOfferId>& OfferIds,
		const TArray<FString>& ItemMappingIds,
		const FString& Error,
		const FUniqueNetIdPtr UserId);

	void OnSyncPurchaseCompleted(
		bool bWasSuccessful, 
		const FString& ErrorMessage);

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineStoreV2AccelBytePtr StoreInterface;
	FOnlineEntitlementsAccelBytePtr EntitlementInterface;
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	bool bIsQueryItemMappingRunning = false;

	FNativeItemPricingMap ItemPriceMap;
	
	FTimerHandle SyncPurchaseTimerHandle;

#pragma region "Steam"
#if PLATFORM_STEAM
	void QuerySteamItemDefinition();
	
	const TMap<FString, FNativeItemPrice>& GetSteamItemPricing(
		const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings);
	
	void OnSteamRequestPricesCompleted(
		SteamInventoryRequestPricesResult_t* ResultCallback, 
		bool bIoFailure);
	
	CCallResult<UNativePlatformPurchaseSubsystem, SteamInventoryRequestPricesResult_t> InventoryRequestPricesResult{};
	STEAM_CALLBACK(UNativePlatformPurchaseSubsystem, HandleSteamPurchaseOverlay, GameOverlayActivated_t);
	TMap<int32, uint64> SteamItemPrices;
#endif
#pragma endregion

#pragma region "Google Play"
#if PLATFORM_ANDROID
	bool ParseGooglePlayReceiptToSyncRequest(
		const TSharedRef<FPurchaseReceipt>& Receipt, 
		FAccelByteModelsPlatformSyncMobileGoogle& OutRequest);

	IOnlineSubsystem* GooglePlaySubsystem;
#endif
#pragma endregion

#pragma region "Utilities"
	FString GetNativePlatformName() const;

	FString GetItemsMappingId(
		const FString ProductId, 
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const;

	const TMap<EAccelBytePlatformType, EAccelBytePlatformMapping> SupportedNativePlatform =
	{
		{ EAccelBytePlatformType::Steam, EAccelBytePlatformMapping::STEAM },
		{ EAccelBytePlatformType::Google, EAccelBytePlatformMapping::GOOGLE },
		{ EAccelBytePlatformType::GooglePlayGames, EAccelBytePlatformMapping::GOOGLE },
		{ EAccelBytePlatformType::EpicGames, EAccelBytePlatformMapping::EPIC_GAMES },
		{ EAccelBytePlatformType::PS4, EAccelBytePlatformMapping::PLAYSTATION },
		{ EAccelBytePlatformType::PS4CrossGen, EAccelBytePlatformMapping::PLAYSTATION },
		{ EAccelBytePlatformType::PS5, EAccelBytePlatformMapping::PLAYSTATION },
		{ EAccelBytePlatformType::Live, EAccelBytePlatformMapping::XBOX }
	};

	const TArray<EAccelByteItemType> SupportedNativePlatformItem = { EAccelByteItemType::COINS };
#pragma endregion 
};