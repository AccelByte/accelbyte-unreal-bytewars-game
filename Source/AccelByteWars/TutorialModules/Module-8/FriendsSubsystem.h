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
	void FindFriend(const APlayerController* PC, const FString& InKeyword, const FOnFindFriendComplete& OnComplete = FOnFindFriendComplete());
	void SendFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnSendFriendRequestComplete& OnComplete = FOnSendFriendRequestComplete());

protected:
	void GetCacheFriendList(const APlayerController* PC, const FOnCacheFriendsDataComplete& OnComplete = FOnCacheFriendsDataComplete());

	void OnFindFriendComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayName, const FUniqueNetId& FoundUserId, const FString& Error, int32 LocalUserNum, const FOnFindFriendComplete OnComplete);
	void OnSendFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete);
#pragma endregion

#pragma region Module.8b Function Declarations
public:
	void BindOnCachedFriendsDataUpdated(const APlayerController* PC, const FOnCachedFriendsDataUpdated& Delegate);
	void UnbindOnCachedFriendsDataUpdated(const APlayerController* PC);

	void GetInboundFriendRequestList(const APlayerController* PC, const FOnGetInboundFriendRequestListComplete& OnComplete= FOnGetInboundFriendRequestListComplete());
	void GetOutboundFriendRequestList(const APlayerController* PC, const FOnGetOutboundFriendRequestListComplete& OnComplete = FOnGetOutboundFriendRequestListComplete());

	void AcceptFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnAcceptFriendRequestComplete& OnComplete = FOnAcceptFriendRequestComplete());
	void RejectFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRejectFriendRequestComplete& OnComplete = FOnRejectFriendRequestComplete());
	void RemoveFriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRemoveFriendComplete& OnComplete = FOnRemoveFriendComplete());

protected:
	void OnAcceptFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnAcceptFriendRequestComplete OnComplete);
	void OnRejectFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRejectFriendRequestComplete OnComplete);
	void OnRemoveFriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRemoveFriendComplete OnComplete);
#pragma endregion

#pragma region Module.8c Function Declarations
public:
	void GetFriendList(const APlayerController* PC, const FOnGetFriendListComplete& OnComplete = FOnGetFriendListComplete());
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FUniqueNetIdPtr GetPlayerUniqueNetId(const APlayerController* PC) const;
	int32 GetPlayerControllerId(const APlayerController* PC) const;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
	FDelegateHandle OnRejectFriendRequestCompleteDelegateHandle;
	FDelegateHandle OnRemoveFriendCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnFriendsChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
};