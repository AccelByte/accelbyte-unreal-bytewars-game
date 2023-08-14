// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogPeriodicLeaderboard, Log, All);

#define UE_LOG_PERIODIC_LEADERBOARD(Verbosity, Format, ...) \
{ \
	UE_LOG(LogPeriodicLeaderboard, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}