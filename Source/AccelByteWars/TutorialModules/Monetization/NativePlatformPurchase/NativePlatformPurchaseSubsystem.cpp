// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativePlatformPurchaseSubsystem.h"

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
}

void UNativePlatformPurchaseSubsystem::QueryItemMapping(const APlayerController* PlayerController)
{
	if (bIsQueryItemMappingRunning)
	{
		return;
	}

	const FUniqueNetIdPtr UserId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!UserId.IsValid())
	{
		return;
	}

	bIsQueryItemMappingRunning = true;

	FString StoreId = TEXT("");
	FString ViewId = TEXT("");
	FString Region = TEXT("");
	constexpr EAccelBytePlatformMapping PlatformMapping = EAccelBytePlatformMapping::STEAM;
	StoreInterface->QueryStorefront(
		*UserId.Get(),
		StoreId,
		ViewId,
		Region,
		PlatformMapping,
		FOnQueryStorefrontComplete::CreateUObject(this, &ThisClass::OnQueryItemMappingComplete));
}

TArray<TSharedRef<FAccelByteModelsItemMapping>> UNativePlatformPurchaseSubsystem::GetItemMapping()
{
	TArray<TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>> Mapping = {};
	StoreInterface->GetItemMappings(Mapping);
	return Mapping;
}

void UNativePlatformPurchaseSubsystem::OpenPlatformStore(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const
{
	const IOnlineSubsystem* PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
	ensure(PlatformSubsystem);

	if (PlatformSubsystem->GetSubsystemName().IsEqual(TEXT("Steam")))
	{
		OpenSteamStore(OwningPlayer, StoreItemData, SelectedPriceIndex, ItemMapping, PlatformSubsystem);
	}
}

void UNativePlatformPurchaseSubsystem::OnQueryItemMappingComplete(
	bool bWasSuccessful,
	const TArray<FString>& ViewIds,
	const TArray<FString>& SectionIds,
	const TArray<FUniqueOfferId>& OfferIds,
	const TArray<FString>& ItemMappingIds,
	const FString& Error)
{
	bIsQueryItemMappingRunning = false;
	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("OnQueryItemMappingComplete: Complete Query Item Mapping with %s"), (bWasSuccessful ? TEXT("Success") : TEXT("Error")));
}

#pragma region "Steam"
void UNativePlatformPurchaseSubsystem::OpenSteamStore(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping,
	const IOnlineSubsystem* PlatformSubsystem) const
{
	FString StoreUrl = TEXT(""); 
	bool isSuccessGetStoreUrl = false;

	isSuccessGetStoreUrl = TryGetSteamStoreUrl(StoreItemData, ItemMapping, StoreUrl);

	if (!isSuccessGetStoreUrl)
	{
		FString mErrorMsg = TEXT("Error when trying to get Steam Store Url");
		OnSynchPurchaseCompleteDelegates.ExecuteIfBound(false, mErrorMsg);

		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error OpenSteamStore: Error when trying to get Steam Store Url"));
		return;
	}

	FAccelByteModelsEntitlementSyncBase EntitlementSyncBase = GetItemToSync(StoreItemData, SelectedPriceIndex, ItemMapping);

	if (EntitlementSyncBase.ProductId.IsEmpty())
	{
		FString mErrorMsg = TEXT("Item is not mapped in AGS Admin Portal");
		OnSynchPurchaseCompleteDelegates.ExecuteIfBound(false, mErrorMsg);

		UE_LOG_NATIVE_PLATFORM_PURCHASE(Error, TEXT("Error OpenSteamStore: Item is not mapped in AGS Admin Portal"));
		return;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	ensure(LocalPlayer != nullptr);
	int32 LocalUserNum = LocalPlayer->GetControllerId();

	PlatformSubsystem->GetExternalUIInterface()->ShowWebURL(StoreUrl,
		FShowWebUrlParams(),
		FOnShowWebUrlClosedDelegate::CreateWeakLambda(this, [this, LocalUserNum, EntitlementSyncBase](const FString& FinalUrl)
			{
				EntitlementInterface->SyncPlatformPurchase(LocalUserNum, EntitlementSyncBase, OnSynchPurchaseCompleteDelegates);
			}));
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

#pragma endregion 