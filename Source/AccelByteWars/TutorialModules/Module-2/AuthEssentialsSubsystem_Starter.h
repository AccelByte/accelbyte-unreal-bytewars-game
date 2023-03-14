// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AuthEssentialsLog.h"
#include "AuthEssentialsSubsystem_Starter.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthOnLoginComplete_Starter, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
typedef FAuthOnLoginComplete_Starter::FDelegate FAuthOnLoginCompleteDelegate_Starter;

UCLASS()
class ACCELBYTEWARS_API UAuthEssentialsSubsystem_Starter : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** Set auth credentials for id/username and token/password. It meant to be used for AccelByte login method.
	 * @param Id Identity of the user logging in (email, display name, facebook id, etc).
	 * @param Token Credentials of the user logging in (password or auth token).
	 */
	void SetAuthCredentials(const FString& Id, const FString& Token);

	/**
	 * Clear auth credentials. It will clear id/username and token/password credentials.
	 * @param bAlsoResetType will clear the auth's login type if it sets to true.
	 */
	void ClearAuthCredentials(bool bAlsoResetType = false);

protected:
	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineAccountCredentials Credentials;
};