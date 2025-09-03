// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "StartupSubsystem.generated.h"

#define GUI_CHEAT_ENTRY_KILL_SELF TEXT("kill-self")
#define GUI_CHEAT_ENTRY_KILL_OTHERS TEXT("kill-others")

DECLARE_DELEGATE_TwoParams(FOnLoginPlatformCompleteDelegate, const bool /*bSucceeded*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnQueryUsersInfoCompleteDelegate, const FOnlineError& /*Error*/, const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& /*UsersInfo*/);

UCLASS()
class ACCELBYTEWARS_API UStartupSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

protected:
	IOnlineUserPtr UserInterface;

#pragma region "Common functions"
public:
	bool CompareAccelByteUniqueId(const FUniqueNetIdRepl& FirstUniqueNetId, const FUniqueNetIdRepl& SecondUniqueNetId) const;
	bool CompareAccelByteUniqueId(const FUniqueNetIdRepl& FirstUniqueNetId, const FString& SecondAbUserId) const;

	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoCompleteDelegate& OnComplete);

protected:
	virtual void OnQueryUserInfoComplete(
		int32 LocalUserNum,
		bool bSucceeded,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FString& ErrorMessage,
		const FOnQueryUsersInfoCompleteDelegate& OnComplete);

private:
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle; 
#pragma endregion 

#pragma region "CLI cheat"
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;

private:
	const FString CommandUserInfo = TEXT("ab.user.UserInfo");
	const FString CommandLobbyConnect = TEXT("ab.lobby.connect");
	const FString CommandLobbyDisconnect = TEXT("ab.lobby.disconnect");

	UFUNCTION()
	void DisplayUserInfo(const TArray<FString>& Args);

public:
	UFUNCTION()
	void LobbyConnect(const TArray<FString>& Args);

	UFUNCTION()
	void LobbyDisconnect(const TArray<FString>& Args);
#pragma endregion 

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
	bool bUseNativeOSSLogin = false;

#pragma endregion

#pragma region "Core game cheats"
protected:
	UFUNCTION()
	void KillSelf(const TArray<FString>& Args);

	UFUNCTION()
	void KillOthers(const TArray<FString>& Args);
#pragma endregion 

#pragma region "Core game helpers"
private:
	void OnQueryTeamMembersInfo(
		const APlayerController* PC,
		const TArray<FUniqueNetIdRef>& MemberUserIds,
		const TDelegate<void(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>& TeamMembersInfo)> OnComplete);
#pragma endregion
};
