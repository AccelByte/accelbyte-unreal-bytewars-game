// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "FriendsEssentialsLog.h"
#include "FriendsEssentialsModels.h"
#include "FriendsSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART FriendsSubsystem.h-public
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "20"]}
// @@@MULTISNIP GetSelfFriendCode {"selectedLines": ["1", "6"]}
// @@@MULTISNIP FindFriend {"selectedLines": ["1", "8"]}
// @@@MULTISNIP SendFriendRequestById {"selectedLines": ["1", "10"]}
// @@@MULTISNIP SendFriendRequestByCode {"selectedLines": ["1", "11"]}
// @@@MULTISNIP GetInboundFriendRequestList {"selectedLines": ["1", "18"]}
// @@@MULTISNIP GetOutboundFriendRequestList {"selectedLines": ["1", "19"]}
// @@@MULTISNIP AcceptFriendRequest {"selectedLines": ["1", "21"]}
// @@@MULTISNIP RejectFriendRequest {"selectedLines": ["1", "22"]}
// @@@MULTISNIP CancelFriendRequest {"selectedLines": ["1", "23"]}
// @@@MULTISNIP OnCachedFriendsDataUpdatedBinders {"selectedLines": ["1", "15-16"]}
// @@@MULTISNIP GetFriendList {"selectedLines": ["1", "29"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Module.8a Function Declarations
	void GetSelfFriendCode(const APlayerController* PC, const FOnGetSelfFriendCodeComplete& OnComplete = FOnGetSelfFriendCodeComplete());

	void FindFriend(const APlayerController* PC, const FString& InKeyword, const FOnFindFriendComplete& OnComplete = FOnFindFriendComplete());

	void SendFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnSendFriendRequestComplete& OnComplete = FOnSendFriendRequestComplete());
	void SendFriendRequest(const APlayerController* PC, const FString& FriendCode, const FOnSendFriendRequestComplete& OnComplete = FOnSendFriendRequestComplete());
#pragma endregion

#pragma region Module.8b Function Declarations
	void BindOnCachedFriendsDataUpdated(const APlayerController* PC, const FOnCachedFriendsDataUpdated& Delegate);
	void UnbindOnCachedFriendsDataUpdated(const APlayerController* PC);

	void GetInboundFriendRequestList(const APlayerController* PC, const FOnGetInboundFriendRequestListComplete& OnComplete = FOnGetInboundFriendRequestListComplete());
	void GetOutboundFriendRequestList(const APlayerController* PC, const FOnGetOutboundFriendRequestListComplete& OnComplete = FOnGetOutboundFriendRequestListComplete());

	void AcceptFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnAcceptFriendRequestComplete& OnComplete = FOnAcceptFriendRequestComplete());
	void RejectFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRejectFriendRequestComplete& OnComplete = FOnRejectFriendRequestComplete());
	void CancelFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnCancelFriendRequestComplete& OnComplete = FOnCancelFriendRequestComplete());

	void GetFriendsInviteStatus(const APlayerController* PC, TArray<UFriendData*> PlayerData, const FOnGetPlayersInviteStatusComplete& OnComplete);
#pragma endregion

#pragma region Module.8c Function Declarations
	void GetFriendList(const APlayerController* PC, const FOnGetFriendListComplete& OnComplete = FOnGetFriendListComplete());
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.h-protected
// @@@MULTISNIP Interface {"selectedLines": ["1", "27-28"]}
// @@@MULTISNIP GetCacheFriendList {"selectedLines": ["1", "3"]}
// @@@MULTISNIP OnFindFriendComplete {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnSendFriendRequestComplete {"selectedLines": ["1", "7"]}
// @@@MULTISNIP OnSendFriendRequestByFriendCodeComplete {"selectedLines": ["1", "8"]}
// @@@MULTISNIP OnAcceptFriendRequestComplete {"selectedLines": ["1", "12"]}
// @@@MULTISNIP OnRejectFriendRequestComplete {"selectedLines": ["1", "13"]}
// @@@MULTISNIP OnCancelFriendRequestComplete {"selectedLines": ["1", "14"]}
protected:
#pragma region Module.8a Function Declarations
	void GetCacheFriendList(const int32 LocalUserNum, bool bQueryUserInfo, const FOnGetCacheFriendListComplete& OnComplete = FOnGetCacheFriendListComplete());

	void OnFindFriendComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayName, const FUniqueNetId& FoundUserId, const FString& Error, int32 LocalUserNum, const FOnFindFriendComplete OnComplete);

	void OnSendFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete);
	void OnSendFriendRequestByFriendCodeComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete);
#pragma endregion

#pragma region Module.8b Function Declarations
	void OnAcceptFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnAcceptFriendRequestComplete OnComplete);
	void OnRejectFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRejectFriendRequestComplete OnComplete);
	void OnCancelFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnCancelFriendRequestComplete OnComplete);
#pragma endregion

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnRejectFriendRequestCompleteDelegateHandle;
	FDelegateHandle OnCancelFriendRequestCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnFriendsChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
// @@@SNIPEND

#pragma region "CLI Cheat"
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;

private:
	const FString CommandReadFriendList = TEXT("ab.friend.readFriendList");

	UFUNCTION()
	void DisplayFriendList(const TArray<FString>& Args);
#pragma endregion 
};
