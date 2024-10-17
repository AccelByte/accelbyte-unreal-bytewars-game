// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "AuthEssentialsModels.generated.h"

#define AUTH_TYPE_ACCELBYTE_PARAM TEXT("ACCELBYTE")
#define AUTH_TYPE_PARAM TEXT("AUTH_TYPE")
#define AUTH_LOGIN_PARAM TEXT("AUTH_LOGIN")
#define AUTH_PASSWORD_PARAM TEXT("AUTH_PASSWORD")
#define PLATFORM_LOGIN_UI_NOT_IMPLEMENTED_CODE TEXT("NOT_IMPLEMENTED")

// @@@SNIPSTART AuthEssentialsModels.h-delegatemacro
// @@@MULTISNIP OnLoginCompleteDelegate {"selectedLines": ["1-2"]}
DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthOnLoginComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
typedef FAuthOnLoginComplete::FDelegate FAuthOnLoginCompleteDelegate;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginSuccess, const APlayerController* /*PlayerController*/);
DECLARE_MULTICAST_DELEGATE(FOnRetryLogin);
// @@@SNIPEND

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

class UAuthEssentialsModels
{
public:
	inline static FOnLoginSuccess OnLoginSuccessDelegate;

	static EAccelBytePlatformType GetPlatformType(FUniqueNetIdPtr NetId)
	{
		if (!ensure(NetId))
		{
			return EAccelBytePlatformType::None;
		}
		const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(NetId);
		if (!ensure(UserABId))
		{
			return EAccelBytePlatformType::None;
		}
		return FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(UserABId->GetPlatformType());
	}
};
