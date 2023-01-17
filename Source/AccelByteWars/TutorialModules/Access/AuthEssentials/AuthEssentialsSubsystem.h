// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "AuthEssentialsSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FAuthOnLoginComplete, bool, bWasSuccessful, FString, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FAuthOnLogoutComplete, bool, bWasSuccessful);

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** Login user using specified login method */
	UFUNCTION(BlueprintCallable)
	void Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FAuthOnLoginComplete& OnLoginComplete);

	UFUNCTION(BlueprintCallable)
	void Logout(const APlayerController* PC, const FAuthOnLogoutComplete& OnLogoutComplete);

	UFUNCTION(BlueprintCallable)
	bool IsLoggedIn(const APlayerController* PC);

	/** Set auth credentials for id/username and token/password. It meant to be used for AccelByte login method. */
	UFUNCTION(BlueprintCallable)
	void SetAuthCredentials(const FString& Id, const FString& Token);

	/**
	 * Clear auth credentials. It will clear id/username and token/password credentials.
	 * @param bAlsoResetType will clear the auth's login type if it sets to true.
	 */
	void ClearAuthCredentials(bool bAlsoResetType = false);

	UFUNCTION(BlueprintCallable)
	bool IsAccelByteSDKInitialized();

protected:
	void OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FAuthOnLoginComplete OnLoginComplete);
	void OnLogoutComplete(int32 LocalUserNum, bool bLogoutWasSuccessful, const FAuthOnLogoutComplete OnLogoutComplete);
	int32 GetLocalUserNum(const APlayerController* PC);

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;
};