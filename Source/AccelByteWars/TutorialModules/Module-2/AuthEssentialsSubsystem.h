// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "AuthEssentialsSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthOnLoginComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
typedef FAuthOnLoginComplete::FDelegate FAuthOnLoginCompleteDelegate;

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** Login user using specified login method */
	void Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FAuthOnLoginCompleteDelegate& OnLoginComplete);

	/** Set auth credentials for id/username and token/password. It meant to be used for AccelByte login method.
	 * @param Id Identity of the user logging in (email, display name, facebook id, etc).
	 * @param Token Credentials of the user logging in (password or auth token).
	 */
	void SetAuthCredentials(const FString& Id, const FString& Token);

	/**
	 * Clear auth credentials. It will clear id/username and token/password credentials.
	 * @param bAlsoResetType will clear the auth's login type if it sets to true.
	 */
	void ClearAuthCredentials(bool bAlsoResetType = false);

protected:
	void OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FAuthOnLoginCompleteDelegate OnLoginComplete);

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;
};