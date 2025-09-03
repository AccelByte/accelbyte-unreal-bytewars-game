// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Online/CoreOnlineFwd.h"
#include "Models/AccelByteEcommerceModels.h"
#include "NativePlatformPurchaseModels.generated.h"

UENUM()
enum EPurchaseState : uint8
{
	NotStarted = 0,
	Purchasing,
	SyncInProgress,
	Completed,
	Failed
};

USTRUCT()
struct FNativeItemPrice
{
	GENERATED_BODY()
	FString Id{};
	uint64 Price{};
};

#define NATIVE_PURCHASE_TIMEOUT 5.f
using FNativeItemPricingMap = TMap<FString, FNativeItemPrice>;

DECLARE_DELEGATE_OneParam(FOnQueryItemMappingCompleted, const FNativeItemPricingMap& /*Pricing*/);
DECLARE_DELEGATE_TwoParams(FOnQueryItemMapping, const FUniqueNetIdPtr /*UserId*/, FOnQueryItemMappingCompleted /*OnCompleted*/);
DECLARE_DELEGATE_RetVal(const FNativeItemPricingMap&, FOnGetItemPrices);

class ACCELBYTEWARS_API FNativePlatformPurchaseUtils
{
public:
	static inline FOnQueryItemMapping OnQueryItemMapping;
	static inline FOnGetItemPrices OnGetItemPrices;
	static inline FString RegionalCurrencyCode;
};

static inline TArray<TSharedRef<FAccelByteModelsItemMapping>> GooglePlayItemMapping =
{
	MakeShared<FAccelByteModelsItemMapping>(FAccelByteModelsItemMapping{
		EAccelBytePlatformMapping::GOOGLE,
		TEXT("sku"),
		TEXT("unreal-bytewars-gem-100"),
		TEXT("bytewars_gem_100")}),
};