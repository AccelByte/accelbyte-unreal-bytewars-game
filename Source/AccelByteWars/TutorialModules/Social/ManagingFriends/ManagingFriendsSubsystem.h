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

UCLASS()
class ACCELBYTEWARS_API UManagingFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.12 Function Declarations
public:
	void GetBlockedPlayerList(const APlayerController* PC, const FOnGetBlockedPlayerListComplete& OnComplete = FOnGetBlockedPlayerListComplete());

	void BindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC, const FOnGetCacheBlockedPlayersDataUpdated& Delegate);
	void UnbindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC);

	void Unfriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete = FOnUnfriendComplete());
	void BlockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete = FOnBlockPlayerComplete());
	void UnblockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete = FOnUnblockPlayerComplete());

protected:
	void OnUnfriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnUnfriendComplete OnComplete);
	void OnBlockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnBlockPlayerComplete OnComplete);
	void OnUnblockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnUnblockPlayerComplete OnComplete);
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

protected:
	void OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete = FOnUnfriendComplete());
	void OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete = FOnBlockPlayerComplete());

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FDelegateHandle OnUnfriendCompleteDelegateHandle;
	FDelegateHandle OnQueryBlockedPlayersCompleteDelegateHandle;
	FDelegateHandle OnBlockPlayerCompleteDelegateHandle;
	FDelegateHandle OnUnblockPlayerCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	TMap<int32, FDelegateHandle> OnBlockedPlayersChangeDelegateHandles;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
};
