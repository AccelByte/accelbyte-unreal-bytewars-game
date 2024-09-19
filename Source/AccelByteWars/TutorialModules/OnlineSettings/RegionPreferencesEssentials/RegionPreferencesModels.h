// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "RegionPreferencesModels.generated.h"

#define BYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"
#define LATENCY_TEXT_FMT NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Latency Text", "{0} ms")
#define WARNING_MINIMUM_REGION NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Minimum region count", "You must enable at least one region!!!")
#define INGAME_REGION_TEXT_FMT NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Ingame Region Text", "Region: {0}")
#define INGAME_LATENCY_TEXT_FMT NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Ingame Latency Text", "Latency: {0}")
#define REGION_OPT_IN_TXT TEXT("OPT IN")
#define REGION_OPT_OUT_TXT TEXT("OPT OUT")
#define REGION_UNKNOWN TEXT("UNKNOWN")

UCLASS()
class ACCELBYTEWARS_API URegionPreferenceInfo final : public UObject
{
	GENERATED_BODY()
public:
	FString RegionCode = TEXT("");
	float Latency = 0.0f;
	bool bEnabled = true;
};
