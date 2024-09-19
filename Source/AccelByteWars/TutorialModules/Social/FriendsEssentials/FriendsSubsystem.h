// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "FriendsEssentialsLog.h"
#include "FriendsEssentialsModels.h"
#include "FriendsSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

#pragma region Module.8a Function Declarations
public:
	void GetSelfFriendCode(const APlayerController* PC, const FOnGetSelfFriendCodeComplete& OnComplete = FOnGetSelfFriendCodeComplete());

	void FindFriend(const APlayerController* PC, const FString& InKeyword, const FOnFindFriendComplete& OnComplete = FOnFindFriendComplete());

	void SendFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnSendFriendRequestComplete& OnComplete = FOnSendFriendRequestComplete());
	void SendFriendRequest(const APlayerController* PC, const FString& FriendCode, const FOnSendFriendRequestComplete& OnComplete = FOnSendFriendRequestComplete());

protected:
	void GetCacheFriendList(const int32 LocalUserNum, const FOnGetCacheFriendListComplete& OnComplete = FOnGetCacheFriendListComplete());

	void OnFindFriendComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayName, const FUniqueNetId& FoundUserId, const FString& Error, int32 LocalUserNum, const FOnFindFriendComplete OnComplete);

	void OnSendFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete);
	void OnSendFriendRequestByFriendCodeComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete);


#pragma endregion

#pragma region Module.8b Function Declarations
public:
	void BindOnCachedFriendsDataUpdated(const APlayerController* PC, const FOnCachedFriendsDataUpdated& Delegate);
	void UnbindOnCachedFriendsDataUpdated(const APlayerController* PC);

	void GetInboundFriendRequestList(const APlayerController* PC, const FOnGetInboundFriendRequestListComplete& OnComplete= FOnGetInboundFriendRequestListComplete());
	void GetOutboundFriendRequestList(const APlayerController* PC, const FOnGetOutboundFriendRequestListComplete& OnComplete = FOnGetOutboundFriendRequestListComplete());

	void AcceptFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnAcceptFriendRequestComplete& OnComplete = FOnAcceptFriendRequestComplete());
	void RejectFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRejectFriendRequestComplete& OnComplete = FOnRejectFriendRequestComplete());
	void CancelFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnCancelFriendRequestComplete& OnComplete = FOnCancelFriendRequestComplete());

protected:
	void OnAcceptFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnAcceptFriendRequestComplete OnComplete);
	void OnRejectFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRejectFriendRequestComplete OnComplete);
	void OnCancelFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnCancelFriendRequestComplete OnComplete);
#pragma endregion

#pragma region Module.8c Function Declarations
public:
	void GetFriendList(const APlayerController* PC, const FOnGetFriendListComplete& OnComplete = FOnGetFriendListComplete());
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnRejectFriendRequestCompleteDelegateHandle;
	FDelegateHandle OnCancelFriendRequestCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnFriendsChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;

#pragma region "CLI Cheat"
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;

private:
	const FString CommandReadFriendList = TEXT("ab.friend.readFriendList");

	UFUNCTION()
	void DisplayFriendList(const TArray<FString>& Args);
#pragma endregion 
};