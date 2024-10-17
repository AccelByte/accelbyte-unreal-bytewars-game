// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

// @@@SNIPSTART AuthEssentialsSubsystem.h-include
// @@@MULTISNIP OnlineIdentityInterfaceAccelByte {"selectedLines": ["2"]}
#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "AuthEssentialsModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "AuthEssentialsSubsystem.generated.h"
// @@@SNIPEND

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART AuthEssentialsSubsystem.h-public
// @@@MULTISNIP Login {"selectedLines": ["1", "6"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** Login user using specified login method */
	void Login(const APlayerController* PC, const FAuthOnLoginCompleteDelegate& OnLoginComplete);

	/** Set auth credentials, including login method, id/username, and token/password.
	 * @param Login method (e.g. Device Id, AccelByte, etc). Set to EAccelByteLoginType::None if login with default native platform.
	 * @param Id Identity of the user logging in (email, display name, facebook id, etc).
	 * @param Token Credentials of the user logging in (password or auth token).
	 */
	void SetAuthCredentials(const EAccelByteLoginType& LoginMethod, const FString& Id, const FString& Token);

	/**
	 * Clear auth credentials. It will clear login method, id/username, and token/password.
	 */
	void ClearAuthCredentials();

	TSharedPtr<FUserOnlineAccountAccelByte> GetLoggedInUserOnlineAccount(const int LocalUserIndex = 0) const;
	
	TSharedPtr<FUserOnlineAccountAccelByte> GetOrQueryLoggedInUserOnlineAccount();
// @@@SNIPEND
	
// @@@SNIPSTART AuthEssentialsSubsystem.h-protected
// @@@MULTISNIP IdentityInterface {"selectedLines": ["1", "4"]}
// @@@MULTISNIP Credentials {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnLoginComplete {"selectedLines": ["1", "2"]}
protected:
	void OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FAuthOnLoginCompleteDelegate OnLoginComplete);

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;
// @@@SNIPEND

#pragma region "CLI Cheat"
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;

private:
	const FString CommandMyUserInfo = TEXT("ab.user.MyUserInfo");

	UFUNCTION()
	void DisplayMyUserInfo(const TArray<FString>& Args) const;
#pragma endregion 
};