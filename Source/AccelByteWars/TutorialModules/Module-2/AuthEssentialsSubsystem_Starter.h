// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "AuthEssentialsSubsystem_Starter.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthOnLoginComplete_Starter, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
typedef FAuthOnLoginComplete_Starter::FDelegate FAuthOnLoginCompleteDelegate_Starter;

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** Login user using specified login method */
	void Login(const APlayerController* PC, const FAuthOnLoginCompleteDelegate_Starter& OnLoginComplete);

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

protected:
	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;
};