// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Social/FriendsEssentials/FriendsEssentialsModels.h"
#include "ManagingFriendsLog.h"
#include "ManagingFriendsSubsystem.generated.h"

class UPromptSubsystem;
class UFriendsSubsystem;
class UCommonActivatableWidget;

UCLASS()
class ACCELBYTEWARS_API UManagingFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART ManagingFriendsSubsystem.h-public
// @@@MULTISNIP GetBlockedPlayerList {"selectedLines": ["1", "9"]}
// @@@MULTISNIP Unfriend {"selectedLines": ["1", "14"]}
// @@@MULTISNIP BlockPlayer {"selectedLines": ["1", "15"]}
// @@@MULTISNIP UnblockPlayer {"selectedLines": ["1", "16"]}
// @@@MULTISNIP OnCachedBlockedPlayersDataUpdatedBinders {"selectedLines": ["1", "11-12"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	void SetGeneratedWidgetButtonsVisibility(UFriendData* PlayerData);
	void BindGeneratedWidgetButtonsAction(const APlayerController* PlayerController, UFriendData* PlayerData);
	void UnbindGeneratedWidgetButtonsAction();

#pragma region Module.12 Function Declarations
	void GetBlockedPlayerList(const APlayerController* PC, bool bQueryUserInfo, const FOnGetBlockedPlayerListComplete& OnComplete = FOnGetBlockedPlayerListComplete());

	void BindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC, const FOnGetCacheBlockedPlayersDataUpdated& Delegate);
	void UnbindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC);

	void Unfriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete = FOnUnfriendComplete());
	void BlockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete = FOnBlockPlayerComplete());
	void UnblockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete = FOnUnblockPlayerComplete());
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.h-protected
// @@@MULTISNIP Interface {"selectedLines": ["1", "31-32"]}
// @@@MULTISNIP OnUnfriendComplete {"selectedLines": ["1", "3"]}
// @@@MULTISNIP OnBlockPlayerComplete {"selectedLines": ["1", "4"]}
// @@@MULTISNIP OnUnblockPlayerComplete {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnUnfriendButtonClicked {"selectedLines": ["1", "9"]}
// @@@MULTISNIP OnBlockButtonClicked {"selectedLines": ["1", "10"]}
protected:
	void OnQueryBlockedPlayersComplete(const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error, const FOnGetBlockedPlayerListComplete OnComplete);
	void OnUnfriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnUnfriendComplete OnComplete);
	void OnBlockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnBlockPlayerComplete OnComplete);
	void OnUnblockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnUnblockPlayerComplete OnComplete);
#pragma endregion

	void UpdateFriendStatus(const APlayerController* PlayerController, UFriendData* PlayerData);
	void OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete = FOnUnfriendComplete());
	void OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete = FOnBlockPlayerComplete());
	void OnUnblockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl UnblockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete = FOnUnblockPlayerComplete());
	void OnInviteAsFriendButtonClicked(const APlayerController* PC, UFriendData* PlayerData, const FOnInviteAsFriendPlayerComplete& OnComplete = FOnInviteAsFriendPlayerComplete());
	void OnAcceptFriendRequestButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl PlayerUserId, const FOnAcceptFriendRequestComplete& OnComplete = FOnAcceptFriendRequestComplete());
	void OnRejectFriendRequestButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl PlayerUserId, const FOnRejectFriendRequestComplete& OnComplete = FOnRejectFriendRequestComplete());
	void OnCancelFriendRequestButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl PlayerUserId, const FOnCancelFriendRequestComplete& OnComplete = FOnCancelFriendRequestComplete());

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnUnfriendCompleteDelegateHandle;
	FDelegateHandle OnQueryBlockedPlayersCompleteDelegateHandle;
	FDelegateHandle OnBlockPlayerCompleteDelegateHandle;
	FDelegateHandle OnUnblockPlayerCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnBlockedPlayersChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;
	UFriendsSubsystem* FriendsSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
// @@@SNIPEND
};
