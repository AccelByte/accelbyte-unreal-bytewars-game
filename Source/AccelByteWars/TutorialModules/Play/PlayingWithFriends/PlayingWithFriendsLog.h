// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogPlayingWithFriends, Log, All);

#define UE_LOG_PLAYINGWITHFRIENDS(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogPlayingWithFriends, Verbosity, Format, ##__VA_ARGS__) \
}
