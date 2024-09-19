// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "AuthEssentialsModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "AuthEssentialsSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthOnLoginComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
typedef FAuthOnLoginComplete::FDelegate FAuthOnLoginCompleteDelegate;

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

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

protected:
	void OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FAuthOnLoginCompleteDelegate OnLoginComplete);

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;

#pragma region "CLI Cheat"
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;

private:
	const FString CommandMyUserInfo = TEXT("ab.user.MyUserInfo");

	UFUNCTION()
	void DisplayMyUserInfo(const TArray<FString>& Args) const;
#pragma endregion 
};