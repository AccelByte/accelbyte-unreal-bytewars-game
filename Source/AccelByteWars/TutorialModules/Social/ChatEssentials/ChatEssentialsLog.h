// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogChatEssentials, Log, All);

#define UE_LOG_CHATESSENTIALS(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogChatEssentials, Verbosity, Format, ##__VA_ARGS__) \
}