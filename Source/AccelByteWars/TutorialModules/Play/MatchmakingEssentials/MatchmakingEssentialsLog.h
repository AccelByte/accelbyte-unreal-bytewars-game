// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogMatchmakingEssentials, Log, All);

#define UE_LOG_MATCHMAKING(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogMatchmakingEssentials, Verbosity, Format, ##__VA_ARGS__) \
}