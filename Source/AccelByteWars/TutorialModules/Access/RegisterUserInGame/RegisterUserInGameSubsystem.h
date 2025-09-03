// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegisterUserInGameModels.h"
#include "RegisterUserInGameLog.h"
#include "RegisterUserInGameSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API URegisterUserInGameSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART RegisterUserInGameSubsystem.h-public
// @@@MULTISNIP ValidateUserInput {"selectedLines": ["1", "5-7"]}
// @@@MULTISNIP UpgradeAndVerifyAccount {"selectedLines": ["1", "9-17"]}
// @@@MULTISNIP SendUpgradeAccountVerificationCode {"selectedLines": ["1", "19-22"]}
// @@@MULTISNIP IsCurrentUserIsFullAccount {"selectedLines": ["1", "24"]}
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ValidateUserInput(
		const FUserInputValidationRequest& Request,
		const FOnUserInputValidationComplete& OnComplete = FOnUserInputValidationComplete());

	void UpgradeAndVerifyAccount(
		const int32 LocalUserNum,
		const FUniqueNetIdPtr UserId,
		const FString& Username,
		const FString& DisplayName,
		const FString& EmailAddress,
		const FString& Password,
		const FString& VerificationCode,
		const FOnUpgradeAndVerifyAccountComplete& OnComplete = FOnUpgradeAndVerifyAccountComplete());

	void SendUpgradeAccountVerificationCode(
		const FString& EmailAddress,
		const bool bForceResend = false,
		const FOnSendUpgradeAccountVerificationCodeComplete& OnComplete = FOnSendUpgradeAccountVerificationCodeComplete());

	bool IsCurrentUserIsFullAccount();
	bool IsAllowUpgradeAccount();
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.h-protected
// @@@MULTISNIP OnUpgradeAndVerifyAccountSuccess {"selectedLines": ["1-6"]}
protected:
	void OnUpgradeAndVerifyAccountSuccess(
		const FAccountUserData& NewFullAccount,
		const int32 LocalUserNum,
		const FUniqueNetIdRef UserId,
		const FOnUpgradeAndVerifyAccountComplete OnComplete);
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.h-private
// @@@MULTISNIP LastVerificationCodeTargetEmails {"selectedLines": ["1", "6"]}
private:
	void GetUpgradeAccountConfig();

	bool bAllowUpgradeAccount = false;

	TSet<FString> LastVerificationCodeTargetEmails;
// @@@SNIPEND
};
