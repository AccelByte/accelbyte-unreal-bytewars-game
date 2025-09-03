// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

#include "StoreItemModel.generated.h"

#define TEXT_PRICE_FREE NSLOCTEXT("AccelByteWars", "free", "Free")
#define TEXT_PURCHASE NSLOCTEXT("AccelByteWars", "purchase-with", "Purchase with")

#define TEXT_CURRENCY_CONFIG_SECTION TEXT("CurrencyCode")
#define TEXT_CURRENCY_CONFIG_COIN TEXT("Coin")
#define TEXT_CURRENCY_CONFIG_GEM TEXT("Gem")

#define TEXT_CURRENCY_CONFIG_COIN_DEFAULT TEXT("BC")
#define TEXT_CURRENCY_CONFIG_GEM_DEFAULT TEXT("BG")

// For native currency, prices are stored as integers in cents format.
// Divide by 100 (or multiply by 0.01) to convert to dollars. Example: 25 = $0.25
// Regional pricing follows the same conversion logic.
#define FORMAT_NATIVE_CURRENCY(InAmount, InCurrencyCode)\
	FText::FromString(FString::Printf(TEXT("%s %.2f"), *InCurrencyCode, InAmount * 0.01f))

// @@@SNIPSTART StoreItemModel.h-CurrencyType
UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	NONE = 0,
	NATIVE,
	COIN,
	GEM
};
// @@@SNIPEND

// @@@SNIPSTART StoreItemModel.h-PreConfigCurrency
// @@@MULTISNIP GetCodeFromType {"selectedLines": ["1-4", "59"]}
// @@@MULTISNIP GetTypeFromCode {"selectedLines": ["1-3", "34", "59"]}
class FPreConfigCurrency
{
public:
	static FString GetCodeFromType(const ECurrencyType CurrencyType)
	{
		FString ConfigKey;
		FString DefaultCurrencyCode;

		switch (CurrencyType)
		{
		case ECurrencyType::COIN:
			ConfigKey = TEXT_CURRENCY_CONFIG_COIN;
			DefaultCurrencyCode = TEXT_CURRENCY_CONFIG_COIN_DEFAULT;
			break;
		case ECurrencyType::GEM:
			ConfigKey = TEXT_CURRENCY_CONFIG_GEM;
			DefaultCurrencyCode = TEXT_CURRENCY_CONFIG_GEM_DEFAULT;
			break;
		default:
			// Return empty string immediately
			return TEXT("");
		}

		FString CurrencyCode;
		if (!GConfig->GetString(TEXT_CURRENCY_CONFIG_SECTION, *ConfigKey, CurrencyCode, GEngineIni))
		{
			// No config found in DefaultEngine.ini, use default value
			return DefaultCurrencyCode;
		}

		return CurrencyCode;
	}

	static ECurrencyType GetTypeFromCode(const FString& CurrencyCode)
	{
		FString CoinCurrencyCode;
		if (!GConfig->GetString(TEXT_CURRENCY_CONFIG_SECTION, TEXT_CURRENCY_CONFIG_COIN, CoinCurrencyCode, GEngineIni))
		{
			CoinCurrencyCode = TEXT_CURRENCY_CONFIG_COIN_DEFAULT;
		}

		FString GemCurrencyCode;
		if (!GConfig->GetString(TEXT_CURRENCY_CONFIG_SECTION, TEXT_CURRENCY_CONFIG_GEM, GemCurrencyCode, GEngineIni))
		{
			GemCurrencyCode = TEXT_CURRENCY_CONFIG_GEM_DEFAULT;
		}

		if (CurrencyCode == CoinCurrencyCode)
		{
			return ECurrencyType::COIN;
		}
		if (CurrencyCode == GemCurrencyCode)
		{
			return ECurrencyType::GEM;
		}

		return ECurrencyType::NONE;
	}
};
// @@@SNIPEND

// @@@SNIPSTART StoreItemModel.h-StoreItemPriceDataObject
// @@@MULTISNIP Data {"selectedLines": ["1-3", "33-41", "45"]}
UCLASS(BlueprintType)
class UStoreItemPriceDataObject : public UObject
{
	GENERATED_BODY()

public:
	void Setup(
		const ECurrencyType InCurrencyType,
		const int64 InRegularPrice,
		const int64 InFinalPrice)
	{
		CurrencyType = InCurrencyType;
		RegularPrice = InRegularPrice;
		FinalPrice = InFinalPrice;
	}
	
	void Setup(
		const ECurrencyType InCurrencyType,
		const int64 InRegularPrice,
		const int64 InFinalPrice,
		const FString& InCurrencyCode)
	{
		Setup(InCurrencyType, InRegularPrice, InFinalPrice);
		NativeCurrencyCode = InCurrencyCode;
	}

	ECurrencyType GetCurrencyType() const { return CurrencyType; }
	int64 GetRegularPrice() const { return RegularPrice; }
	int64 GetFinalPrice() const { return FinalPrice; }
	bool IsDiscounted() const { return RegularPrice != FinalPrice; }
	const FString& GetNativeCurrencyCode() const { return NativeCurrencyCode; }

private:
	UPROPERTY(EditAnywhere)
	ECurrencyType CurrencyType;

	UPROPERTY(EditAnywhere)
	int64 RegularPrice;

	UPROPERTY(EditAnywhere)
	int64 FinalPrice;

	UPROPERTY(EditAnywhere)
	FString NativeCurrencyCode;
};
// @@@SNIPEND

UCLASS()
class UStoreItemDataObject : public UObject
{
	GENERATED_BODY()

private:
	FText Title;
	FText Category;
	FString ItemType;
	FString StoreItemId;
	FString EntitlementId;
	FString IconUrl;

	TMap<EItemSkuPlatform, FString> SkuMap;

	UPROPERTY()
	TArray<UStoreItemPriceDataObject*> Prices;

	int32 Count;
	bool bConsumable;

	// Byte Wars specifics, used for the core UI to determine whether to show the prices or not
	bool bShouldShowPrices = true;

public:

	void Setup(
		const FText& InTitle,
		const FText& InCategory,
		const FString& InItemType,
		const FString& InStoreItemId,
		const FString& InEntitlementId,
		const FString& InIconUrl,
		const TMap<EItemSkuPlatform, FString>& InSkuMap,
		const TArray<UStoreItemPriceDataObject*> InPrices,
		const int32 InCount,
		const bool InIsConsumable)
	{
		Title = InTitle;
		Category = InCategory;
		ItemType = InItemType;
		StoreItemId = InStoreItemId;
		EntitlementId = InEntitlementId;
		IconUrl = InIconUrl;
		SkuMap = InSkuMap;
		Prices = InPrices;
		Count = InCount;
		bConsumable = InIsConsumable;
	}

	void Setup(const UStoreItemDataObject* Other)
	{
		Title = Other->GetTitle();
		Category = Other->GetCategory();
		ItemType = Other->GetItemType();
		StoreItemId = Other->GetStoreItemId();
		EntitlementId = Other->GetEntitlementId();
		IconUrl = Other->GetIconUrl();
		SkuMap = Other->GetSkuMap();
		Prices = Other->GetPrices();
		Count = Other->GetCount();
		bConsumable = Other->GetIsConsumable();
		bShouldShowPrices = Other->GetShouldShowPrices();
	}

	void SetShouldShowPrices(const bool bValue)
	{
		bShouldShowPrices = bValue;
	}

	const FText& GetTitle() const { return Title; }
	const FText& GetCategory() const { return Category; }
	const FString& GetItemType() const { return ItemType; }
	const FString& GetStoreItemId() const { return StoreItemId; }
	const FString& GetEntitlementId() const { return EntitlementId; }
	const FString& GetIconUrl() const { return IconUrl; }
	const TMap<EItemSkuPlatform, FString>& GetSkuMap() const { return SkuMap; }
	const TArray<UStoreItemPriceDataObject*>& GetPrices() const { return Prices; }
	int32 GetCount() const { return Count; }
	bool GetIsConsumable() const { return bConsumable; }
	bool GetShouldShowPrices() const { return bShouldShowPrices; }

	/** @brief Set variables with FOnlineEntitlement's variables */
	void Setup(const FOnlineEntitlement& Entitlement)
	{
		StoreItemId = Entitlement.ItemId;
		Title = FText::FromString(Entitlement.Name);
		bConsumable = Entitlement.bIsConsumable;
		Count = Entitlement.RemainingCount;
		EntitlementId = Entitlement.Id;
	}

	/** @brief Copy variables from other object if variable is empty */
	void UpdateVariables(const UStoreItemDataObject* Other)
	{
		if (!Other)
		{
			return;
		}

		if (Title.IsEmpty())
		{
			Title = Other->Title;
		}

		if (Category.IsEmpty())
		{
			Category = Other->Category;
		}

		if (ItemType.IsEmpty())
		{
			ItemType = Other->ItemType;
		}

		if (StoreItemId.IsEmpty())
		{
			StoreItemId = Other->StoreItemId;
		}

		if (EntitlementId.IsEmpty())
		{
			EntitlementId = Other->EntitlementId;
		}

		if (SkuMap.IsEmpty())
		{
			SkuMap = Other->SkuMap;
		}

		if (IconUrl.IsEmpty())
		{
			IconUrl = Other->IconUrl;
		}

		if (Count <= 0)
		{
			Count = Other->Count;
		}

		if (Prices.IsEmpty())
		{
			Prices = Other->Prices;
		}

		bConsumable = Other->bConsumable;
	}

	FString GetSku(const EItemSkuPlatform Platform) const
	{
		if (SkuMap.Contains(Platform))
		{
			return SkuMap[Platform];
		}

		return FString();
	}
};
