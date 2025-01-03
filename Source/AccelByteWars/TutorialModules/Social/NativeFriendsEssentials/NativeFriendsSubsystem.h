// Copyright (c) 2024 Native AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "NativeFriendsEssentialsLog.h"
#include "NativeFriendsEssentialsModels.h"
#include "NativeFriendsSubsystem.generated.h"

class UFriendsSubsystem;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UNativeFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
#pragma region Module Get Native Friend List Function Declarations
	void GetNativeFriendList(const APlayerController* PC, const FOnGetNativeFriendListComplete& OnComplete);
#pragma endregion

#pragma region Module Sync Native Friend List Function Declarations
	void SyncNativePlatformFriendList(const APlayerController* PC, const FOnSyncNativePlatformFriendListComplete& OnComplete);
#pragma endregion

protected:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineSubsystemAccelByte* ABSubsystem;

	UPromptSubsystem* PromptSubsystem;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineFriendsAccelBytePtr FriendsInterface;
	FOnlineUserCacheAccelBytePtr UserCache;

#pragma region Module Get Native Friend List Function Declarations
	void QueryUsersByPlatformIds(const int32 LocalUserNum, TArray<TSharedRef<FOnlineFriend>> FriendList, const FOnGetNativeFriendListComplete OnComplete);

	void OnQueryUsersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried, const int32 LocalUserNum, const FOnGetNativeFriendListComplete OnComplete);
#pragma endregion

#pragma region Module Sync Native Friend List Function Declarations
	void OnSyncNativePlatformFriendListComplete(int32 LocalUserNum, const FOnlineError& ErrorInfo, const TArray<FAccelByteModelsSyncThirdPartyFriendsResponse>& Response, const FOnSyncNativePlatformFriendListComplete OnComplete);

	FDelegateHandle OnSyncThirdPartyPlatformFriendsV2CompleteDelegateHandle;
#pragma endregion
};
