// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegisterUserInGameSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/AccelByteWebSocketErrorTypes.h"
#include "Api/AccelByteUserApi.h"

void URegisterUserInGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GetUpgradeAccountConfig();
}

// @@@SNIPSTART RegisterUserInGameSubsystem.cpp-Deinitialize
void URegisterUserInGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
	LastVerificationCodeTargetEmails.Empty();
}
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.cpp-UpgradeAndVerifyAccount
void URegisterUserInGameSubsystem::UpgradeAndVerifyAccount(
	const int32 LocalUserNum,
	const FUniqueNetIdPtr UserId,
	const FString& Username,
	const FString& EmailAddress,
	const FString& Password,
	const FString& VerificationCode,
	const FOnUpgradeAndVerifyAccountComplete& OnComplete)
{
	if (!UserId.IsValid()) 
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to upgrade headless account to full account. User ID is null."));
		OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString(), FAccountUserData());
		return;
	}
	
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to upgrade headless account to full account. AccelByte API Client is invalid."));
		OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString(), FAccountUserData());
		return;
	}

	AccelByte::Api::UserPtr UserApi = ApiClient->GetUserApi().Pin();
	if (!UserApi)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to upgrade headless account to full account. User API is invalid."));
		OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString(), FAccountUserData());
		return;
	}

	FUpgradeAndVerifyRequest Request;
	Request.Username = Username;
	Request.DisplayName = Username;
	Request.UniqueDisplayName = Username;
	Request.EmailAddress = EmailAddress;
	Request.Password = Password;
	Request.Code = VerificationCode;

	UserApi->UpgradeAndVerify2(
		Request,
		THandler<FAccountUserData>::CreateUObject(this, &ThisClass::OnUpgradeAndVerifyAccountSuccess, LocalUserNum, UserId.ToSharedRef(), OnComplete),
		FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to upgrade headless account to full account. Error %d: %s"), ErrorCode, *ErrorMessage);
			OnComplete.ExecuteIfBound(false, GetUpgradeAccountErrorMessage((EUpgradeAccountErrorTypes)ErrorCode).ToString(), FAccountUserData());
		})
	);
}
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.cpp-SendUpgradeAccountVerificationCode
void URegisterUserInGameSubsystem::SendUpgradeAccountVerificationCode(
	const FString& EmailAddress, 
	const bool bForceResend, 
	const FOnSendUpgradeAccountVerificationCodeComplete& OnComplete)
{
	if (!bForceResend && LastVerificationCodeTargetEmails.Contains(EmailAddress))
	{
		UE_LOG_REGISTERUSERINGAME(Log, TEXT("Verification code already sent to the same e-mail."));
		OnComplete.ExecuteIfBound(true, TEXT(""));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to send upgrade to full account verification code to the registered e-mail. AccelByte API Client is invalid."));
		OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString());
		return;
	}

	AccelByte::Api::UserPtr UserApi = ApiClient->GetUserApi().Pin();
	if (!UserApi)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to send upgrade to full account verification code to the registered e-mail. User API is invalid."));
		OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString());
		return;
	}

	UserApi->SendUpgradeVerificationCode(
		EmailAddress,
		AccelByte::FVoidHandler::CreateWeakLambda(this, [this, EmailAddress, OnComplete]()
		{
			UE_LOG_REGISTERUSERINGAME(Log, TEXT("Success to send upgrade to full account verification code to the registered e-mail."));
			LastVerificationCodeTargetEmails.Add(EmailAddress);
			OnComplete.ExecuteIfBound(true, TEXT(""));
		}),
		FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to send upgrade to full account verification code to the registered e-mail. Error %d: %s"), ErrorCode, *ErrorMessage);
			OnComplete.ExecuteIfBound(false, GetUpgradeAccountErrorMessage((EUpgradeAccountErrorTypes)ErrorCode).ToString());
		})
	);
}
// @@@SNIPEND

// @@@SNIPSTART RegisterUserInGameSubsystem.cpp-IsCurrentUserIsFullAccount
bool URegisterUserInGameSubsystem::IsCurrentUserIsFullAccount()
{
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Cannot get user full account status. AccelByte API Client is invalid."));
		return false;
	}

	const bool bHasLinkedEmail = !ApiClient->CredentialsRef->GetUserEmailAddress().IsEmpty();
	const bool bIsEmailVerified = ApiClient->CredentialsRef->GetAccountUserData().EmailVerified;
	return bHasLinkedEmail && bIsEmailVerified;
}
// @@@SNIPEND

bool URegisterUserInGameSubsystem::IsAllowUpgradeAccount()
{
	return bAllowUpgradeAccount;
}

// @@@SNIPSTART RegisterUserInGameSubsystem.cpp-OnUpgradeAndVerifyAccountSuccess
void URegisterUserInGameSubsystem::OnUpgradeAndVerifyAccountSuccess(
	const FAccountUserData& NewFullAccount, 
	const int32 LocalUserNum,
	const FUniqueNetIdRef UserId, 
	const FOnUpgradeAndVerifyAccountComplete OnComplete)
{
	UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>();
	if (!StartupSubsystem) 
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to handle on upgrade headless account success. Startup subsystem is null."));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to handle on upgrade headless account success.. AccelByte API Client is invalid."));
		return;
	}

	// Query new user info to update account user cache on OSS.
	StartupSubsystem->QueryUserInfo(
		LocalUserNum,
		TPartyMemberArray{ UserId },
		FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, ApiClient, LocalUserNum, UserId, NewFullAccount, OnComplete](
			const FOnlineError& Error,
			const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
			{
				if (!Error.bSucceeded || UsersInfo.IsEmpty())
				{
					UE_LOG_REGISTERUSERINGAME(
						Warning, 
						TEXT("Failed to upgrade headless account to full account. Error %s: %s"), 
						*Error.ErrorCode, 
						*Error.ErrorMessage.ToString());
					OnComplete.ExecuteIfBound(false, UPGRADE_ACCOUNT_UNKNOWN_ERROR.ToString(), FAccountUserData());
					return;
				}

				UE_LOG_REGISTERUSERINGAME(Log, TEXT("Success to upgrade headless account to full account: %s"), *NewFullAccount.DisplayName);

				// Update account user cache on SDK.
				ApiClient->CredentialsRef->SetAccountUserData(NewFullAccount);

				OnComplete.ExecuteIfBound(true, TEXT(""), NewFullAccount);
			}));
}
// @@@SNIPEND

void URegisterUserInGameSubsystem::GetUpgradeAccountConfig()
{
	const FString CmdArgs = FCommandLine::Get();
	bool bIsValidCmdValue = false;

	// Check allow upgrade account option launch parameter.
	const FString CmdStr = FString("-AllowUpgradeAccount=");
	if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
	{
		FString CmdValue = TEXT("");
		FParse::Value(*CmdArgs, *CmdStr, CmdValue);
		if (!CmdValue.IsEmpty())
		{
			bAllowUpgradeAccount = CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			bIsValidCmdValue = true;
			UE_LOG_REGISTERUSERINGAME(Log, TEXT("Launch param sets the allow upgrade account config to %s."), bAllowUpgradeAccount ? TEXT("TRUE") : TEXT("FALSE"));
		}
	}
	if (!bIsValidCmdValue)
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bAllowUpgradeAccount"), bAllowUpgradeAccount, GEngineIni);
		UE_LOG_REGISTERUSERINGAME(Log, TEXT("DefaultEngine.ini sets the allow upgrade account config to %s."), bAllowUpgradeAccount ? TEXT("TRUE") : TEXT("FALSE"));
	}
}
