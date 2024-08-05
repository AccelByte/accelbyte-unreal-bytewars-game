// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthEssentialsModels.generated.h"

#define AUTH_TYPE_ACCELBYTE_PARAM TEXT("ACCELBYTE")
#define AUTH_TYPE_PARAM TEXT("AUTH_TYPE")
#define AUTH_LOGIN_PARAM TEXT("AUTH_LOGIN")
#define AUTH_PASSWORD_PARAM TEXT("AUTH_PASSWORD")
#define PLATFORM_LOGIN_UI_NOT_IMPLEMENTED_CODE TEXT("NOT_IMPLEMENTED")

/** @brief login state enumeration. */
enum class ELoginState 
{
	Default,
	LoggingIn,
	Failed
};

UENUM(BlueprintType)
enum EAuthStatus
{
	NotLoggedIn,
	UsingLocalProfile,
	LoggedIn
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginSuccess, const APlayerController* /*PlayerController*/);
DECLARE_MULTICAST_DELEGATE(FOnRetryLogin);

class UAuthEssentialsModels
{
public:
	inline static FOnLoginSuccess OnLoginSuccessDelegate;
};