// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Api/AccelByteUserApi.h"
#include "RegisterUserInGameModels.generated.h"

USTRUCT()
struct ACCELBYTEWARS_API FUpgradeAccountData
{
	GENERATED_BODY()

public:
	FUpgradeAccountData() {}
	FUpgradeAccountData(const FString& InUsername, const FString& InDisplayName, const FString& InEmail, const FString& InPassword)
		: Username(InUsername), DisplayName(InDisplayName), Email(InEmail), Password(InPassword) {}

	FString GetUsername() const { return Username; }
	FString GetDisplayName() const { return DisplayName; }
	FString GetEmail() const { return Email; }
	FString GetPassword() const { return Password; }

	void Reset()
	{
		Username = TEXT("");
		DisplayName = TEXT("");
		Email = TEXT("");
		Password = TEXT("");
	}

private:
	FString Username = TEXT("");
	FString DisplayName = TEXT("");
	FString Email = TEXT("");
	FString Password = TEXT("");
};

UENUM()
enum class EUpgradeAccountErrorTypes : int32
{
	Default = 0,
	EmailAlreadyUsed = 10133,
	UsernameAlreadyUsed = 10177,
	UniqueDisplayNameAlreadyUsed = 10222
};

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

// @@@SNIPSTART RegisterUserInGameModels.h-stringmacro
#define SEND_VERIFICATION_CODE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Send Verification Code Message", "Sending verification code")
#define RESEND_VERIFICATION_CODE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Send Verification Code Message", "Re-send Verification Code")
#define UPGRADE_ACCOUNT_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Message", "Upgrading and verifying account")
#define EMPTY_REQUIRED_FIELDS_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Empty Required Fields Error", "Required fields cannot be empty")
#define EMPTY_VERIFICATION_CODE_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Empty Verification Code Error", "Verification code cannot be empty")
#define EMAIL_INPUT_VIOLATION_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Email Input Violation Error", "E-mail format is invalid")
#define EMAIL_ALREADY_USED_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Email Already Used Error", "E-mail is already used")
#define USERNAME_ALREADY_USED_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Username Already Used Error", "Username is already used")
#define DISPLAYNAME_ALREADY_USED_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Display Name Already Used Error", "Display Name is already used")
#define PASSWORD_NOT_MATCH_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Password Not Match Error", "Password does not match, retype the password")
#define UPGRADE_ACCOUNT_UNKNOWN_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Unknown Error", "Unknown error occurred. Please try again")
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameModels.h-delegatemacro
DECLARE_DELEGATE_ThreeParams(FOnUpgradeAndVerifyAccountComplete, bool bWasSuccessful, const FString& ErrorMessage, const FAccountUserData& NewFullAccount);
DECLARE_DELEGATE_TwoParams(FOnSendUpgradeAccountVerificationCodeComplete, bool bWasSuccessful, const FString& ErrorMessage);
DECLARE_DELEGATE_TwoParams(FOnUserInputValidationComplete, bool bIsValid, const FString& ValidationMessage);
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameModels.h-GetUpgradeAccountErrorMessage
inline const FText GetUpgradeAccountErrorMessage(const EUpgradeAccountErrorTypes ErrorCode)
{
	switch ((EUpgradeAccountErrorTypes)ErrorCode)
	{
	case EUpgradeAccountErrorTypes::EmailAlreadyUsed:
		return EMAIL_ALREADY_USED_ERROR;
	case EUpgradeAccountErrorTypes::UsernameAlreadyUsed:
		return USERNAME_ALREADY_USED_ERROR;
	case EUpgradeAccountErrorTypes::UniqueDisplayNameAlreadyUsed:
		return DISPLAYNAME_ALREADY_USED_ERROR;
	default:
		return UPGRADE_ACCOUNT_UNKNOWN_ERROR;
	}
}
// @@@SNIPEND
