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
    FUpgradeAccountData(const FString& InUsername, const FString& InEmail, const FString& InPassword)
        : Username(InUsername), Email(InEmail), Password(InPassword) {}

    FString GetUsername() const { return Username; }
    FString GetEmail() const { return Email; }
    FString GetPassword() const { return Password; }

    void Reset()
    {
        Username = TEXT("");
        Email = TEXT("");
        Password = TEXT("");
    }

private:
    FString Username = TEXT("");
    FString Email = TEXT("");
    FString Password = TEXT("");
};

UENUM()
enum class EUpgradeAccountErrorTypes : int32
{
    Default = 0,
    EmailAlreadyUsed = 10133,
    UsernameAlreadyUsed = 10177,
    UniqueDisplayNameAlreadyUsed = 10222,
    UsernameInputViolation = 20001,
    PasswordInputViolation = 20002
};

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

// @@@SNIPSTART RegisterUserInGameModels.h-stringmacro
#define SEND_VERIFICATION_CODE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Send Verification Code Message", "Sending verification code")
#define RESEND_VERIFICATION_CODE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Send Verification Code Message", "Re-send Verification Code")
#define UPGRADE_ACCOUNT_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Message", "Upgrading and verifying account")
#define EMPTY_REQUIRED_FIELDS_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Empty Required Fields Error", "Required fields cannot be empty")
#define EMPTY_VERIFICATION_CODE_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Empty Verification Code Error", "Verification code cannot be empty")
#define EMAIL_ALREADY_USED_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Email Already Used Error", "E-mail is already used")
#define EMAIL_INPUT_VIOLATION_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Email Input Violation Error", "E-mail format is invalid")
#define USERNAME_ALREADY_USED_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Username Already Used Error", "Username is already used")
#define USERNAME_INPUT_VIOLATION_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Username Input Validation Error", "Username must be {0}-{1} characters and cannot contain whitespaces nor special characters")
#define PASSWORD_INPUT_VIOLATION_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Password Input Violation Error", "Password must be {0}-{1} characters, must contains a mix of upercase and lowercase letters, and cannot contain special characters except dash and underscore")
#define PASSWORD_NOT_MATCH_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Password Not Match Error", "Password does not match, retype the password")
#define UPGRADE_ACCOUNT_UNKNOWN_ERROR NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Upgrade Account Unknown Error", "Unknown error occurred")
// @@@SNIPEND

#define MIN_USERNAME_LEN 5
#define MAX_USERNAME_LEN 20

#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 16

// @@@SNIPSTART RegisterUserInGameModels.h-delegatemacro
DECLARE_DELEGATE_ThreeParams(FOnUpgradeAndVerifyAccountComplete, bool bWasSuccessful, const FString& ErrorMessage, const FAccountUserData& NewFullAccount);
DECLARE_DELEGATE_TwoParams(FOnSendUpgradeAccountVerificationCodeComplete, bool bWasSuccesful, const FString& ErrorMessage);
// @@@SNIPEND

inline bool IsValidUsername(const FString& Username)
{
    const bool bIsValidLength = Username.Len() >= MIN_USERNAME_LEN && Username.Len() <= MAX_USERNAME_LEN;

    // Cannot contain whitespaces
    FRegexPattern WhiteSpacesPattern(TEXT("\\s"));
    bool bHasNoWhiteSpaces = !FRegexMatcher(WhiteSpacesPattern, Username).FindNext();

    // Cannot contain special characters
    FRegexPattern NoSpecialCharsPattern(TEXT("^[a-zA-Z0-9]+$"));
    bool bHasNoSpecialChars = FRegexMatcher(NoSpecialCharsPattern, Username).FindNext();

    return bIsValidLength && bHasNoWhiteSpaces && bHasNoSpecialChars;
};

inline bool IsValidPassword(const FString& Password)
{
    const bool bIsValidLength = Password.Len() >= MIN_PASSWORD_LEN && Password.Len() <= MAX_PASSWORD_LEN;

    // Cannot contain whitespaces
    FRegexPattern WhiteSpacesPattern(TEXT("\\s"));
    bool bHasNoWhiteSpaces = !FRegexMatcher(WhiteSpacesPattern, Password).FindNext();

    // Must contain uppercase and lowercase letter
    FRegexPattern UpperCasePattern(TEXT("[A-Z]"));
    FRegexPattern LowerCasePattern(TEXT("[a-z]"));
    bool bHasUpperCase = FRegexMatcher(UpperCasePattern, Password).FindNext();
    bool bHasLowerCase = FRegexMatcher(LowerCasePattern, Password).FindNext();

    // Cannot contain special characters, except _ and -
    FRegexPattern NoSpecialCharsPattern(TEXT("^[a-zA-Z0-9_-]+$"));
    bool bHasNoSpecialChars = FRegexMatcher(NoSpecialCharsPattern, Password).FindNext();

    return bIsValidLength && bHasNoWhiteSpaces && bHasNoSpecialChars && bHasUpperCase && bHasLowerCase;
};

// @@@SNIPSTART RegisterUserInGameModels.h-GetUpgradeAccountErrorMessage
inline const FText GetUpgradeAccountErrorMessage(const EUpgradeAccountErrorTypes ErrorCode)
{
    switch ((EUpgradeAccountErrorTypes)ErrorCode)
    {
    case EUpgradeAccountErrorTypes::EmailAlreadyUsed:
        return EMAIL_ALREADY_USED_ERROR;
        break;
    case EUpgradeAccountErrorTypes::UsernameAlreadyUsed:
    case EUpgradeAccountErrorTypes::UniqueDisplayNameAlreadyUsed:
        return USERNAME_ALREADY_USED_ERROR;
        break;
    case EUpgradeAccountErrorTypes::UsernameInputViolation:
        return FText::Format(USERNAME_INPUT_VIOLATION_ERROR, MIN_USERNAME_LEN, MAX_USERNAME_LEN);
        break;
    case EUpgradeAccountErrorTypes::PasswordInputViolation:
        return FText::Format(PASSWORD_INPUT_VIOLATION_ERROR, MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        break;
    default:
        return UPGRADE_ACCOUNT_UNKNOWN_ERROR;
        break;
    }
}
// @@@SNIPEND
