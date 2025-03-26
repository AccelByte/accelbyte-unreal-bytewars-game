// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativePlatformPurchaseSubsystem.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

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
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mapping because it is already running."));
		return;
	}

	const FUniqueNetIdPtr UserId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!UserId.IsValid())
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mapping. The User Id is invalid."));
		return;
	}

	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(GetNativePlatformName());
	if (!SupportedNativePlatform.Contains(PlatformType))
	{
		UE_LOG_NATIVE_PLATFORM_PURCHASE(Warning, TEXT("Failed to query item mapping. Cannot find item mapping for %s platform."), *GetNativePlatformName());
		return;
	}

	bIsQueryItemMappingRunning = true;

	const EAccelBytePlatformMapping PlatformMapping = SupportedNativePlatform[PlatformType];
	const FString StoreId = TEXT(""), ViewId = TEXT(""), Region = TEXT("");
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
	const FString& Error)
{
	bIsQueryItemMappingRunning = false;
	UE_LOG_NATIVE_PLATFORM_PURCHASE(Log, TEXT("OnQueryItemMappingComplete: Complete Query Item Mapping with %s"), (bWasSuccessful ? TEXT("Success") : TEXT("Error")));
}

#pragma region "Steam"
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
			EntitlementInterface->SyncPlatformPurchase(LocalUserNum, EntitlementSyncBase, OnSynchPurchaseCompleteDelegates);
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