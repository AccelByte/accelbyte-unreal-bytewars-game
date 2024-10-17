// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogNativeFriendsEssentials, Log, All);

#define UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Verbosity, Format, ...) \
{ \
	UE_LOG(LogNativeFriendsEssentials, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}
