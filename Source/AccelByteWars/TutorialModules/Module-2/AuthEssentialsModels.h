// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

/** @brief login state enumeration. */
enum class ELoginState 
{
	Default,
	LoggingIn,
	Failed
};

DECLARE_DELEGATE(FOnLoginWithSinglePlatformAuth);