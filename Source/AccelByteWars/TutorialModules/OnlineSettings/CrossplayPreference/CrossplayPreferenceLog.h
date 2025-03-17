// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogCrossplayPreference, Log, All);

#define UE_LOG_CROSSPLAY_PREFERENCES(Verbosity, Format, ...) \
{ \
UE_LOG_FUNC(LogCrossplayPreference, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}