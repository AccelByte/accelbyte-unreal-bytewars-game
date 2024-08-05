// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "StartupLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "StartupSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FOnLoginPlatformCompleteDelegate, const bool /*bSucceeded*/, const FString& /*ErrorMessage*/);

UCLASS()
class ACCELBYTEWARS_API UStartupSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Login with Platform Only

public:
	// Login with platform only without consuming the token to login to AccelByte.
	void LoginPlatformOnly(const APlayerController* PC, const FOnLoginPlatformCompleteDelegate& OnLoginComplete);

	// Login to AccelByte using platform token.
	void LoginPlatformCredsToAccelByte(const APlayerController* PC, const FOnLoginPlatformCompleteDelegate& OnLoginComplete);

	const FOnlineAccountCredentials GetPlatformCredentials() const { return PlatformCredentials; };
	const bool IsAutoUseTokenForABLogin() const { return bAutoUseTokenForABLogin; }

private:
	void InitializePlatformLogin();
	void DeinitializePlatformLogin();

	void CheckAutoUseTokenForABLogin();
	
	void OverrideSinglePlatformAuthButtonAction();
	void OnLoginWithSinglePlatformAuthButtonClicked();

	void OnLoginPlatformOnlyComplete(
		int32 LocalUserNum, 
		bool bSucceeded, 
		const FUniqueNetId& UserId, 
		const FString& LoginErrorMessage,
		const FOnLoginPlatformCompleteDelegate OnLoginComplete);

	void OnLoginPlatformCredsToAccelByteComplete(
		int32 LocalUserNum, 
		bool bSucceeded, 
		const FUniqueNetId& UserId, 
		const FString& LoginErrorMessage,
		const FOnLoginPlatformCompleteDelegate OnLoginComplete);

	IOnlineSubsystem* PlatformSubsystem;
	FOnlineIdentityAccelBytePtr AccelByteIdentityInterface;
	IOnlineIdentityPtr PlatformIdentityInterface;
	EAccelByteLoginType PlatformType;
	FOnlineAccountCredentials PlatformCredentials;

	bool bPlatformAccountSelected = false;
	bool bAutoUseTokenForABLogin = true;

#pragma endregion
};
