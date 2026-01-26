// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativePlatformPurchaseSubsystem.h"

#include "Algo/Transform.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"

DEFINE_LOG_CATEGORY(LogNativePlatformPurchaseSubsystem);

void UNativePlatformPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);

	const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
	ensure(IdentityPtr);
	IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(IdentityPtr);
	ensure(IdentityInterface);

	const IOnlineStoreV2Ptr StorePtr = Subsystem->GetStoreV2Interface();
	ensure(StorePtr);
	StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(StorePtr);
	ensure(StoreInterface);

	const IOnlineEntitlementsPtr EntitlementsPtr = Subsystem->GetEntitlementsInterface();
	ensure(EntitlementsPtr);
	EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(EntitlementsPtr);
	ensure(EntitlementInterface);
	
	const IOnlinePurchasePtr PurchasePtr = Subsystem->GetPurchaseInterface();
	ensure(PurchasePtr);
	PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(PurchasePtr);
	ensure(PurchaseInterface);

	OnSyncPurchaseCompleteDelegates.BindUObject(this, &ThisClass::OnSyncPurchaseCompleted);

	if (!FNativePlatformPurchaseUtils::OnQueryItemMapping.IsBoundToObject(this)) {
		FNativePlatformPurchaseUtils::OnQueryItemMapping.BindUObject(this, &ThisClass::QueryItemMapping);
	}

	if (!FNativePlatformPurchaseUtils::OnGetItemPrices.IsBoundToObject(this)) {
		FNativePlatformPurchaseUtils::OnGetItemPrices.BindUObject(this, &UNativePlatformPurchaseSubsystem::GetItemPrices);
	}

	if (IdentityInterface) 
	{
		// Sync all platform items when connected to lobby
		IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0, FOnConnectLobbyCompleteDelegate::CreateWeakLambda(this, [this](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
		{
			if (bWasSuccessful) 
			{
				SyncAllEntitlements(UserId.AsShared());
			}
		}));
	}

#if PLATFORM_STEAM
	QuerySteamItemDefinition();
#endif

#if PLATFORM_ANDROID
	GooglePlaySubsystem = IOnlineSubsystem::Get(GOOGLEPLAY_SUBSYSTEM);
	ensure(GooglePlaySubsystem);
#endif
}

void UNativePlatformPurchaseSubsystem::Deinitialize()
{
	OnSyncPurchaseCompleteDelegates.Unbind();
	FNativePlatformPurchaseUtils::OnQueryItemMapping.Unbind();
	FNativePlatformPurchaseUtils::OnGetItemPrices.Unbind();

	if (IdentityInterface) 
	{
		IdentityInterface->ClearOnConnectLobbyCompleteDelegates(0, this);
	}
}

void UNativePlatformPurchaseSubsystem::SyncAllEntitlements(
	const FUniqueNetIdPtr UserId, 
	const FOnRequestCompleted& OnComplete,
	const FAccelByteModelsEntitlementSyncBase& EntitlementSyncBase)
{
	FString ErrorMessage = TEXT("");

	if (!UserId) 
	{
		ErrorMessage = TEXT("Failed to sync all platform items. User ID is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	int32 LocalUserNum = 0;
	if (!IdentityInterface->GetLocalUserNum(UserId.ToSharedRef().Get(), LocalUserNum)) 
	{
		ErrorMessage = TEXT("Failed to sync all platform items. LocalUserNum is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Sycn DLCs and purchases from the native platform.
	EntitlementInterface->SyncDLC(
		UserId.ToSharedRef().Get(), 
		FOnRequestCompleted::CreateWeakLambda(this, [this, LocalUserNum, EntitlementSyncBase, OnComplete](bool bIsSycnDLCSuccess, const FString& Error)
		{
			if (bIsSycnDLCSuccess)
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to sync all platform DLCs."));
			}
			else 
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to sync platform DLCs. Error: %s"), *Error);
			}

			EntitlementInterface->SyncPlatformPurchase(
				LocalUserNum,
				EntitlementSyncBase,
				FOnRequestCompleted::CreateWeakLambda(this, [bIsSycnDLCSuccess, OnComplete](bool bIsSyncPurchaseSuccess, const FString& Error)
				{
					if (bIsSyncPurchaseSuccess)
					{
						UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to sync all platform purchases."));
					}
					else
					{
						UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to sync platform purchases. Error: %s"), *Error);
					}

					OnComplete.ExecuteIfBound(bIsSycnDLCSuccess && bIsSyncPurchaseSuccess, Error);
				}));
		}));
}

void UNativePlatformPurchaseSubsystem::QueryItemMapping(
	const FUniqueNetIdPtr UserId, 
	const FOnQueryItemMappingCompleted OnQueryCompleted)
{	
	if (bIsQueryItemMappingRunning)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mapping because it is already running."));
		return;
	}

	bIsQueryItemMappingRunning = true;
	OnQueryItemMappingCompleted = OnQueryCompleted;

	if (!UserId)
	{
		const FString ErrorMessage = TEXT("Failed to query item mapping. User ID is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnQueryItemMappingComplete(false, {}, {}, {}, {}, ErrorMessage, UserId);
		return;
	}

	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(GetNativePlatformName());
	if (!SupportedNativePlatform.Contains(PlatformType))
	{
		const FString ErrorMessage = FString::Printf(TEXT("Failed to query item mapping. Cannot find item mapping for %s platform."), *GetNativePlatformName());
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnQueryItemMappingComplete(false, {}, {}, {}, {}, ErrorMessage, UserId);
		return;
	}

#if PLATFORM_ANDROID
	/* AccelByte does not support Google Play Games Services (GPGS) item mapping
	 * As alternative, the item mapping for GPGS is hardcoded in the NativePlatformPurchaseModels. */
	OnQueryItemMappingComplete(true, {}, {}, {}, {}, TEXT(""), UserId);
	return;
#endif

	const EAccelBytePlatformMapping PlatformMapping = SupportedNativePlatform[PlatformType];
	const FString StoreId = TEXT(""), ViewId = TEXT(""), Region = TEXT("");
	StoreInterface->QueryStorefront(
		UserId.ToSharedRef().Get(),
		StoreId,
		ViewId,
		Region,
		PlatformMapping,
		FOnQueryStorefrontComplete::CreateUObject(this, &ThisClass::OnQueryItemMappingComplete, UserId));
}

void UNativePlatformPurchaseSubsystem::QueryItemsWithNativePlatform(
	const FUniqueNetIdPtr UserId,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings)
{
	IOnlineSubsystem* PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
#if PLATFORM_ANDROID
	PlatformSubsystem = GooglePlaySubsystem;
#endif
	if (!PlatformSubsystem)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using native platform. Subsystem is invalid."));
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	// Check whether the native platform is supported by the game or not.
	const FString PlatformName = GetNativePlatformName();
	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(PlatformName);
	if (!SupportedNativePlatform.Contains(PlatformType))
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Platform is not supported."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	// Select the item mapping type based on the native platform.
	const EAccelBytePlatformMapping PlatformMappingType = SupportedNativePlatform[PlatformType];

	// Get the native platform interfaces.
	const IOnlineStoreV2Ptr PlatformStore = PlatformSubsystem->GetStoreV2Interface();
	if (!PlatformStore)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Store interface is invalid."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	const IOnlineIdentityPtr PlatformIdentity = PlatformSubsystem->GetIdentityInterface();
	if (!PlatformIdentity)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Identity interface is invalid."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	// Get the native platform user.
	int32 LocalUserNum = 0;
	if (!IdentityInterface->GetLocalUserNum(UserId.ToSharedRef().Get(), LocalUserNum))
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. LocalUserNum is invalid."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	const FUniqueNetIdPtr PlatformUniqueId = PlatformIdentity->GetUniquePlayerId(LocalUserNum);
	if (!PlatformUniqueId)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Platform unique ID is invalid."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

#if PLATFORM_EOS
	const UEpicGamesStoreItemConfig* EpicItemConfigs = GetDefault<UEpicGamesStoreItemConfig>();
	if (!EpicItemConfigs)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Unable to load UEpicGamesStoreItemConfig."), *PlatformName);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}
#endif

	// Collect the native platform item IDs to query.
	TArray<FString> OfferIds;
	for (const TSharedRef<FAccelByteModelsItemMapping>& ItemMapping : ItemMappings)
	{
		if (ItemMapping->Platform == PlatformMappingType)
		{
#if PLATFORM_EOS
			if (const FString* EpicOfferId = EpicItemConfigs->AudienceOffers.Find(ItemMapping->PlatformProductId))
			{
				OfferIds.Add(*EpicOfferId);
			}
#else
			OfferIds.Add(ItemMapping->PlatformProductId);
#endif
		}
	}

	// Query the native platform items.
	PlatformStore->QueryOffersById(
		PlatformUniqueId.ToSharedRef().Get(),
		OfferIds,
		FOnQueryOnlineStoreOffersComplete::CreateWeakLambda(this, [this, ItemMappings, PlatformName, PlatformStore]
		(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
		{
			if (!bWasSuccessful)
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Error: %s"), *PlatformName, *Error);
				OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
				return;
			}

			if (!PlatformStore)
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Store interface is invalid."), *PlatformName);
				OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
			}

#if PLATFORM_EOS
			const UEpicGamesStoreItemConfig* EpicItemConfigs = GetDefault<UEpicGamesStoreItemConfig>();
			if (!EpicItemConfigs)
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using %s. Unable to load UEpicGamesStoreItemConfig."), *PlatformName);
				OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
				return;
			}
#endif

			ItemPriceMap.Empty();
			for (const TSharedRef<FAccelByteModelsItemMapping>& ItemMapping : ItemMappings)
			{
				FString PlatformProductId = ItemMapping->PlatformProductId;
#if PLATFORM_EOS
				const FString* EpicOfferId = EpicItemConfigs->AudienceOffers.Find(PlatformProductId);
				if (!EpicOfferId)
				{
					continue;
				}
				PlatformProductId = *EpicOfferId;
#endif

				if (TSharedPtr<FOnlineStoreOffer> Item = PlatformStore->GetOffer(PlatformProductId))
				{
					FNativePlatformPurchaseUtils::RegionalCurrencyCode = Item->CurrencyCode;
					ItemPriceMap.Add(ItemMapping->ItemIdentity, { PlatformProductId, (uint64)Item->NumericPrice });
					UE_LOG_NATIVE_PLATFORM_PURCHASE(Log,
						TEXT("Found item pricing from %s for item: %s. Price: %d. Currency: %s"),
						*PlatformName,
						*PlatformProductId,
						Item->NumericPrice,
						*FNativePlatformPurchaseUtils::RegionalCurrencyCode);
				}
			}

			UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to query items using %s"), *PlatformName);
			OnQueryItemMappingCompleted.ExecuteIfBound(ItemPriceMap);
		}));
}

TArray<TSharedRef<FAccelByteModelsItemMapping>> UNativePlatformPurchaseSubsystem::GetItemMapping()
{
#if PLATFORM_ANDROID
	/* AccelByte does not support Google Play Games Services (GPGS) item mapping
	 * As alternative, the item mapping for GPGS is hardcoded in the NativePlatformPurchaseModels. */
	return GooglePlayItemMapping;
#endif

	TArray<TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>> Mapping = {};
	StoreInterface->GetItemMappings(Mapping);
	return Mapping;
}

void UNativePlatformPurchaseSubsystem::CheckoutItem(
	const FUniqueNetIdPtr UserId, 
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData)
{
	FString ErrorMessage = TEXT("");

	if (!UserId)
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. User ID is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	if (!StoreItemData.IsValid()) 
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. Store item data is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const FString* NativeProductId = StoreItemData->GetSkuMap().Find(EItemSkuPlatform::Native);
	if (!NativeProductId)
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. Native product ID is not found.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}
	
	FPurchaseCheckoutRequest CheckoutRequest;
	CheckoutRequest.AddPurchaseOffer(TEXT(""), *NativeProductId, 1);

	PurchaseSyncState = EPurchaseState::Purchasing;

#if PLATFORM_EOS || PLATFORM_ANDROID || \
    (defined(PLATFORM_PS4) && PLATFORM_PS4) || \
    (defined(PLATFORM_PS5) && PLATFORM_PS5) || \
    (defined(PLATFORM_WINGDK) && PLATFORM_WINGDK) || \
    (defined(PLATFORM_XBOXCOMMON) && PLATFORM_XBOXCOMMON) || \
    (defined(PLATFORM_XB1) && PLATFORM_XB1) || \
    (defined(PLATFORM_XBOXONE) && PLATFORM_XBOXONE) || \
    (defined(PLATFORM_XSX) && PLATFORM_XSX)
	// Purchase with native platform.
	CheckoutWithNativePlatform(UserId, CheckoutRequest, StoreItemData->GetIsConsumable());
	return;
#else
	// Purchase with native platform that supported by AccelByte OSS.
	const FAccelByteModelsEntitlementSyncBase EntitlementSyncBase = { *NativeProductId };
	PurchaseInterface->PlatformCheckout(
		UserId.ToSharedRef().Get(), 
		CheckoutRequest,
		FOnPurchaseCheckoutComplete::CreateWeakLambda(this, [this, UserId, EntitlementSyncBase]
		(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
		{
			PurchaseSyncState = EPurchaseState::SyncInProgress;
			SyncAllEntitlements(UserId, OnSyncPurchaseCompleteDelegates, EntitlementSyncBase);

			if(SyncPurchaseTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(SyncPurchaseTimerHandle);
			}
		}));
#endif
}

void UNativePlatformPurchaseSubsystem::CheckoutWithNativePlatform(
	const FUniqueNetIdPtr UserId,
	const FPurchaseCheckoutRequest CheckoutRequest,
	const bool bIsConsumeable)
{
	FString ErrorMessage = TEXT("");

	IOnlineSubsystem* PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
#if PLATFORM_ANDROID
	PlatformSubsystem = GooglePlaySubsystem;
#endif
	if (!PlatformSubsystem)
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. Platform subsystem is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Check whether the native platform is supported by the game or not.
	const FString PlatformName = GetNativePlatformName();
	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(PlatformName);
	if (!SupportedNativePlatform.Contains(PlatformType))
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. Platform is not supported."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Get the native platform interfaces.
	const IOnlineIdentityPtr PlatformIdentity = PlatformSubsystem->GetIdentityInterface();
	if (!PlatformIdentity)
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. Identity interface is invalid."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const IOnlinePurchasePtr PlatformPurchase = PlatformSubsystem->GetPurchaseInterface();
	if (!PlatformPurchase)
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. Purchase interface is invalid."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Get the native platform user.
	if (!UserId)
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. User ID is invalid."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}
	
	int32 LocalUserNum = 0;
	if (!IdentityInterface->GetLocalUserNum(UserId.ToSharedRef().Get(), LocalUserNum))
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. LocalUserNum is invalid."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const FUniqueNetIdPtr PlatformUniqueId = PlatformIdentity->GetUniquePlayerId(LocalUserNum);
	if (!PlatformUniqueId)
	{
		ErrorMessage = FString::Printf(TEXT("Failed to checkout item using %s. Platform unique ID is invalid."), *PlatformName);
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Checkout using the native platform.
	PlatformPurchase->Checkout(
		PlatformUniqueId.ToSharedRef().Get(),
		CheckoutRequest,
		FOnPurchaseCheckoutComplete::CreateWeakLambda(this, [this, LocalUserNum, UserId, PlatformName, bIsConsumeable]
		(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
		{
			if (!Result.bSucceeded)
			{
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to purchase item using %s. Error: %s"), *PlatformName, *Result.ErrorMessage.ToString());
				OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, Result.ErrorMessage.ToString());
				return;
			}

#if PLATFORM_ANDROID
			FAccelByteModelsPlatformSyncMobileGoogle SyncRequest;
			if (!ParseGooglePlayReceiptToSyncRequest(Receipt, SyncRequest))
			{
				const FString ErrorMessage = TEXT("Failed to purchase item using Google Play Games Services. Failed to parse receipt into sync purchase request.");
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
				OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
				return;
			}

			// Request to auto consume for consumables, otherwise request to ack for durable items.
			SyncRequest.AutoConsume = bIsConsumeable;
			SyncRequest.AutoAck = !bIsConsumeable;

			// Sync Google Play Games Services purchase with AccelByte.
			UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to purchase item using %s."), *PlatformName);
			PurchaseSyncState = EPurchaseState::SyncInProgress;
			EntitlementInterface->SyncPlatformPurchase(LocalUserNum, SyncRequest, OnSyncPurchaseCompleteDelegates);
#else
			// Sync checkouts to AccelByte.
			UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to purchase item using %s."), *PlatformName);
			PurchaseSyncState = EPurchaseState::SyncInProgress;
			SyncAllEntitlements(UserId, OnSyncPurchaseCompleteDelegates);
#endif
		}));
}

void UNativePlatformPurchaseSubsystem::OnQueryItemMappingComplete(
	bool bWasSuccessful,
	const TArray<FString>& ViewIds,
	const TArray<FString>& SectionIds,
	const TArray<FUniqueOfferId>& OfferIds,
	const TArray<FString>& ItemMappingIds,
	const FString& Error,
	const FUniqueNetIdPtr UserId)
{
	bIsQueryItemMappingRunning = false;
	if (!bWasSuccessful)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mappings. Error: %s"), *Error);
		OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
		return;
	}

	TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMappings = GetItemMapping();
	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to query item mapping for: %s. Mapped items: %d"), *GetNativePlatformName(), ItemMappings.Num());

	// Use cached item prices if available.
	if (!GetItemPrices().IsEmpty())
	{
		OnQueryItemMappingCompleted.ExecuteIfBound(GetItemPrices());
		return;
	}

#if PLATFORM_STEAM
	OnQueryItemMappingCompleted.ExecuteIfBound(GetSteamItemPricing(ItemMappings));
#else
	QueryItemsWithNativePlatform(UserId, ItemMappings);
#endif
}

void UNativePlatformPurchaseSubsystem::OnSyncPurchaseCompleted(
	bool bWasSuccessful, 
	const FString& ErrorMessage)
{
	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Sync purchase succeed: %s. Error: %s"), bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"), *ErrorMessage);

	if (SyncPurchaseTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SyncPurchaseTimerHandle);
	}
	PurchaseSyncState = bWasSuccessful ? EPurchaseState::Completed : EPurchaseState::Failed;
}

#pragma region "Steam"
#if PLATFORM_STEAM
void UNativePlatformPurchaseSubsystem::QuerySteamItemDefinition()
{
	SteamInventory()->LoadItemDefinitions();
	SteamAPICall_t RequestPricesCall = SteamInventory()->RequestPrices();
	if(RequestPricesCall == k_uAPICallInvalid)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("API ERROR"))
		return;
	}

	InventoryRequestPricesResult.Set(RequestPricesCall, this, &ThisClass::OnSteamRequestPricesCompleted);
}

const TMap<FString, FNativeItemPrice>& UNativePlatformPurchaseSubsystem::GetSteamItemPricing(const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings)
{
	ItemPriceMap.Empty();
	for (const TSharedRef<FAccelByteModelsItemMapping>& ItemMapping : ItemMappings)
	{
		const FString& PlatformProductId = ItemMapping->PlatformProductId;
		if (!PlatformProductId.IsEmpty())
		{
			for(const TPair<int32, uint64>& ItemPrice : SteamItemPrices)
			{
				FString ItemDefIdStr = FString::FromInt(ItemPrice.Key);
				if(ItemDefIdStr.Equals(PlatformProductId))
				{
					ItemPriceMap.Add(ItemMapping->ItemIdentity, {ItemDefIdStr, ItemPrice.Value});
				}
			}
		}
	}
	return ItemPriceMap;
}

void UNativePlatformPurchaseSubsystem::OnSteamRequestPricesCompleted(
	SteamInventoryRequestPricesResult_t* ResultCallback, 
	bool bIoFailure)
{
	if (ResultCallback->m_result == k_EResultOK)
	{
		FNativePlatformPurchaseUtils::RegionalCurrencyCode = ResultCallback->m_rgchCurrency;
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("OnSteamRequestPricesCompleted: completed. Regional Price: %s."), *FNativePlatformPurchaseUtils::RegionalCurrencyCode);

		uint32 ItemDefCount = 0;
		if (SteamInventory()->GetItemDefinitionIDs(nullptr, &ItemDefCount) && ItemDefCount > 0)
		{
			SteamItemPrices.Empty(ItemDefCount);
			TArray<SteamItemDef_t> ItemDefIDs;
			ItemDefIDs.SetNum(ItemDefCount);
    
			if (SteamInventory()->GetItemDefinitionIDs(ItemDefIDs.GetData(), &ItemDefCount))
			{
				for (const SteamItemDef_t& ItemDefID : ItemDefIDs)
				{
					uint64 CurrentPrice, BasePrice;
					SteamInventory()->GetItemPrice(ItemDefID, &CurrentPrice, &BasePrice);
					SteamItemPrices.Add(ItemDefID, CurrentPrice);
					UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Item Definition ID: %d. Price: %llu"), ItemDefID, CurrentPrice);
				}
			}
		}
		
		TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMappings = GetItemMapping();
		if (ItemMappings.Num() > 0)
		{
			OnQueryItemMappingCompleted.ExecuteIfBound(GetSteamItemPricing(ItemMappings));
		}
	}
}

void UNativePlatformPurchaseSubsystem::HandleSteamPurchaseOverlay(GameOverlayActivated_t* pParam)
{
	if(!pParam->m_bActive && PurchaseSyncState == EPurchaseState::Purchasing)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			GetWorld()->GetTimerManager().SetTimer(
				SyncPurchaseTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if(PurchaseSyncState == EPurchaseState::Purchasing)
					{
						OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, TEXT("Canceled"));
					}
				}),
				NATIVE_PURCHASE_TIMEOUT,
				false
			);
		});
	}
}
#endif
#pragma endregion

#pragma region "Google Play"
#if PLATFORM_ANDROID
bool UNativePlatformPurchaseSubsystem::ParseGooglePlayReceiptToSyncRequest(
	const TSharedRef<FPurchaseReceipt>& Receipt,
	FAccelByteModelsPlatformSyncMobileGoogle& OutRequest)
{
	if (Receipt->ReceiptOffers.IsEmpty())
	{
		return false;
	}

	const TArray<FPurchaseReceipt::FLineItemInfo>& LineItems = Receipt->ReceiptOffers[0].LineItems;
	if (LineItems.IsEmpty())
	{
		return false;
	}

	const FString& ValidationInfo = LineItems[0].ValidationInfo;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ValidationInfo);

	TSharedPtr<FJsonObject> JsonObject;
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	FString ReceiptData;
	if (!JsonObject->TryGetStringField(TEXT("receiptData"), ReceiptData))
	{
		return false;
	}

	FString DecodedReceiptData;
	FBase64::Decode(ReceiptData, DecodedReceiptData);

	if (!FAccelByteJsonConverter::JsonObjectStringToUStruct(DecodedReceiptData, &OutRequest))
	{
		return false;
	}

	bool bIsSubscription = false;
	if (JsonObject->TryGetBoolField(TEXT("isSubscription"), bIsSubscription))
	{
		OutRequest.SubscriptionPurchase = bIsSubscription;
	}

	return true;
}
#endif
#pragma endregion

#pragma region "Utilities"
FString UNativePlatformPurchaseSubsystem::GetNativePlatformName() const
{
	FOnlineSubsystemAccelByte* ABSubsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ABSubsystem)
	{
		return TEXT("");
	}

	return ABSubsystem->GetNativePlatformNameString();
}

FString UNativePlatformPurchaseSubsystem::GetItemsMappingId(
	const FString ProductId, 
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const
{
	FString PlatformProductId = TEXT("");

	for (size_t Index = 0; Index < ItemMapping.Num(); Index++)
	{
		if (ItemMapping[Index].Get().ItemIdentity == ProductId)
		{
			PlatformProductId = ItemMapping[Index].Get().PlatformProductId;
			break;
		}
	}

	return PlatformProductId;
}

bool UNativePlatformPurchaseSubsystem::IsNativePlatformSupported(const FUniqueNetIdPtr UserId) const
{
	if (!GetWorld())
	{
		return false;
	}

	const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(UserId);
	if (!UserABId)
	{
		return false;
	}

	// Check if the native platform is valid and if the user's native platform matches the active native platform.
	const EAccelBytePlatformType UserPlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(UserABId->GetPlatformType());
	const EAccelBytePlatformType NativePlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(GetNativePlatformName());
	const bool bIsNativePlatformSupported = SupportedNativePlatform.Contains(NativePlatformType);
	return bIsNativePlatformSupported && UserPlatformType == NativePlatformType;
}
#pragma endregion 