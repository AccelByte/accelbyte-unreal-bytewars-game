// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsSubsystem.generated.h"

class FOnlineAccountCredentialsAccelByte;

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLoginComplete, bool, bWasSuccessful, FString, ErrorMessage);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLogoutComplete, bool, bWasSuccessful);

protected:
	FOnlineAccountCredentialsAccelByte Credentials { EAccelByteLoginType::None, TEXT(""), TEXT("") };

public:
	/** Login user using specified login method */
	UFUNCTION(BlueprintCallable)
	void Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FOnLoginComplete& OnLoginComplete);

	UFUNCTION(BlueprintCallable)
	void Logout(const APlayerController* PC, const FOnLogoutComplete& OnLogoutComplete);

	UFUNCTION(BlueprintCallable)
	bool IsLoggedIn(const APlayerController* PC);

	/** Set auth credentials for id/username and token/password. It meant to be used for AccelByte login method. */
	UFUNCTION(BlueprintCallable)
	void SetAuthCredentials(const FString& Id, const FString& Token);

	/** Clear auth credentials for id/username and token/password. */
	UFUNCTION(BlueprintCallable)
	void ResetAuthCredentials();

protected:
	/** Set auth token credentials for specified platform. It meant to be used for third-party login, such as Steam, EOS, etc. */
	void SetPlatformCredentials(const int32 LocalUserNum, const FString& PlatformName);

	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	FDelegateHandle LoginCompleteDelegate;
	void OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FOnLoginComplete OnLoginComplete);

	FDelegateHandle LogoutCompleteDelegate;
	void OnLogoutComplete(int32 LocalUserNum, bool bLogoutWasSuccessful, const FOnLogoutComplete OnLogoutComplete);
};