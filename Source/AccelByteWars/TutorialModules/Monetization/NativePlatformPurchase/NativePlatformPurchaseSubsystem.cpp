// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativePlatformPurchaseSubsystem.h"

#include "Algo/Transform.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsModel.h"

DEFINE_LOG_CATEGORY(LogNativePlatformPurchaseSubsystem);

void UNativePlatformPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);
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

#if PLATFORM_STEAM
	QuerySteamItemDefinition();
#endif

#if PLATFORM_ANDROID
	IOnlineSubsystem* GooglePlaySubsystem = IOnlineSubsystem::Get(GOOGLEPLAY_SUBSYSTEM);
	ensure(GooglePlaySubsystem);

	GooglePlayIdentityInterface = GooglePlaySubsystem->GetIdentityInterface();
	ensure(GooglePlayIdentityInterface);

	GooglePlayPurchaseInterface = GooglePlaySubsystem->GetPurchaseInterface();
	ensure(GooglePlayPurchaseInterface);

	GooglePlayStoreInterface = GooglePlaySubsystem->GetStoreV2Interface();
	ensure(GooglePlayStoreInterface);
#endif
}

void UNativePlatformPurchaseSubsystem::QueryItemMapping(FUniqueNetIdPtr UserId, FOnQueryItemMappingCompleted OnQueryCompleted)
{	
	if (bIsQueryItemMappingRunning)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mapping because it is already running."));
		return;
	}

	bIsQueryItemMappingRunning = true;
	OnQueryItemMappingCompleted = OnQueryCompleted;

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
		*UserId,
		StoreId,
		ViewId,
		Region,
		PlatformMapping,
		FOnQueryStorefrontComplete::CreateUObject(this, &ThisClass::OnQueryItemMappingComplete, UserId));
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

void UNativePlatformPurchaseSubsystem::CheckoutItem(const APlayerController* OwningPlayer, const TWeakObjectPtr<UStoreItemDataObject> StoreItemData)
{
	FString ErrorMessage = TEXT("");

	const FString* NativeProductId = StoreItemData->GetSkuMap().Find(EItemSkuPlatform::Native);
	if (!NativeProductId)
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. Native product ID is not found.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}
	
	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!LocalPlayer)
	{
		ErrorMessage = TEXT("Failed to checkout item using native platform. Local player is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	
	FPurchaseCheckoutRequest CheckoutRequest;
	CheckoutRequest.AddPurchaseOffer(TEXT(""), *NativeProductId, 1);

	PurchaseSyncState = EPurchaseState::Purchasing;

#if PLATFORM_ANDROID
	/* Use a dedicated function to handle purchases via Google Play Games Services (GPGS).
	 * GPGS has a unique purchase flow that relies on two subsystems: Google and GPGS.
	 * This is necessary because the AccelByte OSS does not natively support this setup.*/
	CheckoutItemWithGooglePlay(OwningPlayer, CheckoutRequest, StoreItemData->GetIsConsumable());
	return;
#endif

	const FAccelByteModelsEntitlementSyncBase EntitlementSyncBase = { *NativeProductId };
	PurchaseInterface->PlatformCheckout(*GetUniqueNetIdFromPlayerController(OwningPlayer), CheckoutRequest,
		FOnPurchaseCheckoutComplete::CreateWeakLambda(this, [this, EntitlementSyncBase, LocalUserNum](const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& /*Receipt*/)
	{
		PurchaseSyncState = EPurchaseState::SyncInProgress;
		EntitlementInterface->SyncPlatformPurchase(LocalUserNum, EntitlementSyncBase, OnSyncPurchaseCompleteDelegates);

		if(SyncPurchaseTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(SyncPurchaseTimerHandle);
		}
	}));
}

bool UNativePlatformPurchaseSubsystem::OpenPlatformStore(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const
{
	const IOnlineSubsystem* PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
	if (!PlatformSubsystem)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to open platform store. The native platform subsystem is invalid."));
		return false;
	}

	// Open the platform store based on the supported platform type.
	EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(GetNativePlatformName());
	switch (PlatformType)
	{
	case EAccelBytePlatformType::Steam:
		return OpenSteamStore(OwningPlayer, StoreItemData, SelectedPriceIndex, ItemMapping, PlatformSubsystem);
	default:
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to open platform store. The native store for %s platform is not yet implemented."), *GetNativePlatformName());
		return false;
	}
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

#if PLATFORM_STEAM
	OnQueryItemMappingCompleted.ExecuteIfBound(GetSteamItemPricing(ItemMappings));
#elif PLATFORM_ANDROID
	// Query native item information from Google Play Games Services (GPGS) if not yet.
	if (!GetItemPrices().IsEmpty()) 
	{
		OnQueryItemMappingCompleted.ExecuteIfBound(GetItemPrices());
	}
	else 
	{
		TArray<FString> GooglePlayItemIds;
		Algo::Transform(ItemMappings, GooglePlayItemIds, [](const TSharedRef<FAccelByteModelsItemMapping>& Item) { return Item->PlatformProductId; });
		QueryGooglePlayItems(
			UserId,
			GooglePlayItemIds,
			FOnQueryOnlineStoreOffersComplete::CreateWeakLambda(this, [this, ItemMappings](
				bool bWasSuccessful,
				const TArray<FUniqueOfferId>& OfferIds,
				const FString& ErrorMessage)
			{
				if (!bWasSuccessful) 
				{
					UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query items using Google Play Games Services. Error: %s"), *ErrorMessage);
					OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
					return;
				}

				UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to query items using Google Play Games Services"));
				OnQueryItemMappingCompleted.ExecuteIfBound(GetGooglePlayItemPricing(ItemMappings));
			}));
	}
#else
	OnQueryItemMappingCompleted.ExecuteIfBound(FNativeItemPricingMap());
#endif
}

#pragma region "Steam"
void UNativePlatformPurchaseSubsystem::OnSyncPurchaseCompleted(bool bWasSuccess, const FString& ErrorMessage)
{
	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Sync purchase succeed: %s. Error: %s"), bWasSuccess ? TEXT("TRUE") : TEXT("FALSE"), *ErrorMessage);

	if(SyncPurchaseTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SyncPurchaseTimerHandle);
	}
    PurchaseSyncState = bWasSuccess ? EPurchaseState::Completed : EPurchaseState::Failed;
}

bool UNativePlatformPurchaseSubsystem::OpenSteamStore(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping,
	const IOnlineSubsystem* PlatformSubsystem) const
{
	FString StoreUrl = TEXT(""); 
	if (!TryGetSteamStoreUrl(StoreItemData, ItemMapping, StoreUrl))
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error OpenSteamStore: Error when trying to get Steam Store Url"));
		return false;
	}

	const FAccelByteModelsEntitlementSyncBase EntitlementSyncBase = GetItemToSync(StoreItemData, SelectedPriceIndex, ItemMapping);
	if (EntitlementSyncBase.ProductId.IsEmpty())
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error OpenSteamStore: Item is not mapped in AGS Admin Portal"));
		return false;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!LocalPlayer) 
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error OpenSteamStore: Local Player is invalid."));
		return false;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	PlatformSubsystem->GetExternalUIInterface()->ShowWebURL(
		StoreUrl,
		FShowWebUrlParams(),
		FOnShowWebUrlClosedDelegate::CreateWeakLambda(this, [this, LocalUserNum, EntitlementSyncBase](const FString& FinalUrl)
		{
			EntitlementInterface->SyncPlatformPurchase(LocalUserNum, EntitlementSyncBase, OnSyncPurchaseCompleteDelegates);
		}));
	return true;
}

bool UNativePlatformPurchaseSubsystem::TryGetSteamStoreUrl(
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping,
	FString& OutSteamStoreUrl) const
{
	bool isSuccess = false;
	int32 SteamAppId = 0;

	FString ItemDefId = GetItemsMappingId(StoreItemData->GetStoreItemId(), ItemMapping);
	if (ItemDefId.IsEmpty())
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error TryGetSteamStoreUrl: Item Definition ID is empty"));
		return isSuccess;
	}

	GConfig->GetInt(TEXT("OnlineSubsystemSteam"), TEXT("SteamDevAppId"), SteamAppId, GEngineIni);
	if (SteamAppId == 0)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error TryGetSteamStoreUrl: SteamAppId is not configured in defaultengine.ini"));
		return isSuccess;
	}

	OutSteamStoreUrl = FString::Printf(TEXT("https://store.steampowered.com/itemstore/%d/detail/%s/?beta=1"), SteamAppId, *ItemDefId);

	isSuccess = true;
	return isSuccess;
}

FAccelByteModelsEntitlementSyncBase UNativePlatformPurchaseSubsystem::GetItemToSync(
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping)const
{
	FAccelByteModelsEntitlementSyncBase EntitlementSyncBase = {};
	
	FString ItemDefId = GetItemsMappingId(StoreItemData->GetStoreItemId(), ItemMapping);
	const UStoreItemPriceDataObject* SelectedPrice = StoreItemData->GetPrices()[SelectedPriceIndex];

	EntitlementSyncBase.ProductId = ItemDefId;
	EntitlementSyncBase.Price = static_cast<int32>(SelectedPrice->GetRegularPrice());
	EntitlementSyncBase.CurrencyCode = FPreConfigCurrency::GetCodeFromType(SelectedPrice->GetCurrencyType());

	return EntitlementSyncBase;
}

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
	SteamInventoryRequestPricesResult_t* ResultCallback, bool bIoFailure)
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
void UNativePlatformPurchaseSubsystem::CheckoutItemWithGooglePlay(
	const APlayerController* OwningPlayer,
	const FPurchaseCheckoutRequest CheckoutRequest,
	const bool bIsConsumeable)
{
	FString ErrorMessage = TEXT("");

	if (!GooglePlayIdentityInterface)
	{
		ErrorMessage = TEXT("Failed to checkout item using Google Play Games Services. Identity interface is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	if (!GooglePlayPurchaseInterface)
	{
		ErrorMessage = TEXT("Failed to checkout item using Google Play Games Services. Purchase interface is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!LocalPlayer)
	{
		ErrorMessage = TEXT("Failed to checkout item using Google Play Games Services. Local Player is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	const FUniqueNetIdPtr UserId = GooglePlayIdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserId)
	{
		ErrorMessage = TEXT("Failed to checkout item using Google Play Games Services. User ID is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	// Purchase item using Google Play Games Services.
	GooglePlayPurchaseInterface->Checkout(
		UserId.ToSharedRef().Get(),
		CheckoutRequest,
		FOnPurchaseCheckoutComplete::CreateWeakLambda(this,
			[this, LocalUserNum, bIsConsumeable]
			(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
			{
				if (!Result.bSucceeded)
				{
					UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to purchase item using Google Play Games Services. Error: %s"), *Result.ErrorMessage.ToString());
					OnSyncPurchaseCompleteDelegates.ExecuteIfBound(false, Result.ErrorMessage.ToString());
					return;
				}
				
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
				UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Success to purchase item using Google Play Games Services. Synching purchase to AccelByte"));
				PurchaseSyncState = EPurchaseState::SyncInProgress;
				EntitlementInterface->SyncPlatformPurchase(LocalUserNum, SyncRequest, OnSyncPurchaseCompleteDelegates);
			})
	);
}

const TMap<FString, FNativeItemPrice>& UNativePlatformPurchaseSubsystem::GetGooglePlayItemPricing(
	const TArray<TSharedRef<FAccelByteModelsItemMapping>>& ItemMappings)
{
	ItemPriceMap.Empty();

	if (!GooglePlayStoreInterface)
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to get item pricing using Google Play Games Services. Store interface is invalid."));
		return ItemPriceMap;
	}

	for (const TSharedRef<FAccelByteModelsItemMapping>& ItemMapping : ItemMappings)
	{
		TSharedPtr<FOnlineStoreOffer> GooglePlayItem = GooglePlayStoreInterface->GetOffer(ItemMapping->PlatformProductId);
		if (GooglePlayItem)
		{
			FNativePlatformPurchaseUtils::RegionalCurrencyCode = GooglePlayItem->CurrencyCode;
			ItemPriceMap.Add(ItemMapping->ItemIdentity, { ItemMapping->PlatformProductId, (uint64)GooglePlayItem->NumericPrice });
			UE_LOG_NATIVE_PLATFORM_PURCHASE(Log,
				TEXT("Found item pricing from Google Play Games Services for item: %s. Price: %d. Currency: %s"),
				*ItemMapping->PlatformProductId,
				GooglePlayItem->NumericPrice,
				*FNativePlatformPurchaseUtils::RegionalCurrencyCode);
		}
	}

	return ItemPriceMap;
}

void UNativePlatformPurchaseSubsystem::QueryGooglePlayItems(
	const FUniqueNetIdPtr UserId,
	const TArray<FUniqueOfferId>& ItemIds,
	const FOnQueryOnlineStoreOffersComplete& OnComplete)
{
	FString ErrorMessage = TEXT("");

	if (!UserId)
	{
		ErrorMessage = TEXT("Failed to query items using Google Play Games Services. User ID is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, {}, ErrorMessage);
		return;
	}

	if (!GooglePlayStoreInterface) 
	{
		ErrorMessage = TEXT("Failed to query items using Google Play Games Services. Store interface is invalid.");
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("%s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, {}, ErrorMessage);
		return;
	}

	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("Query items using Google Play Games Services"));
	GooglePlayStoreInterface->QueryOffersById(UserId.ToSharedRef().Get(), ItemIds, OnComplete);
}

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
FUniqueNetIdPtr UNativePlatformPurchaseSubsystem::GetUniqueNetIdFromPlayerController(
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

FString UNativePlatformPurchaseSubsystem::GetItemsMappingId(const FString ProductId, const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const
{
	FString PlatformProductId = TEXT("");

	for (size_t i = 0; i < ItemMapping.Num(); i++)
	{
		if (ItemMapping[i].Get().ItemIdentity == ProductId)
		{
			PlatformProductId = ItemMapping[i].Get().PlatformProductId;
			break;
		}
	}

	return PlatformProductId;
}

FString UNativePlatformPurchaseSubsystem::GetNativePlatformName() const
{
	FOnlineSubsystemAccelByte* ABSubsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ABSubsystem)
	{
		return TEXT("");
	}

	return ABSubsystem->GetNativePlatformNameString();
}

bool UNativePlatformPurchaseSubsystem::IsNativePlatformSupported() const
{
	if (!GetWorld()) 
	{
		return false;
	}

	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) 
	{
		return false;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) 
	{
		return false;
	}
	
	const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId());
	if (!UserABId) 
	{
		return false;
	}

	// Check if the native platform is valid and if the user's native platform matches the active native platform.
	const EAccelBytePlatformType UserPlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(UserABId->GetPlatformType());
	const EAccelBytePlatformType NativePlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(GetNativePlatformName());
	const bool IsNativePlatformSupported = SupportedNativePlatform.Contains(NativePlatformType);
	return IsNativePlatformSupported && UserPlatformType == NativePlatformType;
}

bool UNativePlatformPurchaseSubsystem::IsItemSupportedByNativePlatform(const FString& Item) const
{
	return SupportedNativePlatformItem.Contains(FAccelByteUtilities::GetUEnumValueFromString<EAccelByteItemType>(Item));
}
#pragma endregion 