// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegisterUserInGameModels.h"
#include "RegisterUserInGameLog.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "RegisterUserInGameSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API URegisterUserInGameSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART RegisterUserInGameSubsystem.h-public
// @@@MULTISNIP UpgradeAndVerifyAccount {"selectedLines": ["1", "5-12"]}
// @@@MULTISNIP SendUpgradeAccountVerificationCode {"selectedLines": ["1", "14-17"]}
// @@@MULTISNIP IsCurrentUserIsFullAccount {"selectedLines": ["1", "19"]}
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void UpgradeAndVerifyAccount(
		const int32 LocalUserNum,
		const FUniqueNetIdPtr UserId,
		const FString& Username,
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
// @@@MULTISNIP Interface {"selectedLines": ["1", "8"]}
protected:
	void OnUpgradeAndVerifyAccountSuccess(
		const FAccountUserData& NewFullAccount,
		const int32 LocalUserNum,
		const FUniqueNetIdRef UserId,
		const FOnUpgradeAndVerifyAccountComplete OnComplete);

	FOnlineUserAccelBytePtr GetUserInterface() const;
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.h-private
// @@@MULTISNIP LastVerificationCodeTargetEmails {"selectedLines": ["1", "6"]}
private:
	void GetUpgradeAccountConfig();

	bool bAllowUpgradeAccount = false;

	TSet<FString> LastVerificationCodeTargetEmails;
// @@@SNIPEND
};
