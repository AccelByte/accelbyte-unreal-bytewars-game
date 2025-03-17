// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineSubsystemUtils.h"
#include "Models/AccelByteEcommerceModels.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
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
		const APlayerController* PlayerController);

	TArray<TSharedRef<FAccelByteModelsItemMapping>> GetItemMapping();

	void OpenPlatformStore(
		const APlayerController* OwningPlayer,
		const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
		const int32 SelectedPriceIndex,
		const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const;
	FOnRequestCompleted OnSynchPurchaseCompleteDelegates;

private:
	FOnlineStoreV2AccelBytePtr StoreInterface;
	FOnlineEntitlementsAccelBytePtr EntitlementInterface;

	bool bIsQueryItemMappingRunning = false;
	void OnQueryItemMappingComplete(
		bool bWasSuccessful,
		const TArray<FString>& ViewIds,
		const TArray<FString>& SectionIds,
		const TArray<FUniqueOfferId>& OfferIds,
		const TArray<FString>& ItemMappingIds,
		const FString& Error);

#pragma region "Steam"
	void OpenSteamStore(const APlayerController* OwningPlayer,
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
#pragma endregion
	
#pragma region "Utilities"
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	FString GetItemsMappingId(const FString ProductId, const TArray<TSharedRef<FAccelByteModelsItemMapping>> ItemMapping) const;
#pragma endregion 
};