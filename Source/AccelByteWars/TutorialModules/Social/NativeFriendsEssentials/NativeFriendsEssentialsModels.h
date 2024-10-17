// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Social/FriendsEssentials/FriendsEssentialsModels.h"
#include "NativeFriendsEssentialsModels.generated.h"

#define FETCH_NATIVE_FRIENDS_MESSAGE NSLOCTEXT("AccelByteWars", "Fetching Native Friends", "Fetching Native Friends")
#define SYNC_IN_PROGRESS_MESSAGE NSLOCTEXT("AccelByteWars", "Sync in progress", "Sync in progress")
#define FRIEND_SYNCED_MESSAGE NSLOCTEXT("AccelByteWars", "Friend Synced", "Friend Synced")
#define SYNC_FAILED_MESSAGE NSLOCTEXT("AccelByteWars", "Sync failed: {0}", "Sync failed: {0}")

UENUM()
enum class ENativeFriendStatus : uint8
{
	AlreadyFriend = 0,
	PendingInbound,
	PendingOutbound,
	Blocked,
	Unknown
};

UCLASS()
class ACCELBYTEWARS_API UNativeFriendData : public UObject
{
	GENERATED_BODY()

public:
	FUniqueNetIdPtr UserId{};
	FString DisplayName{};
	FString AvatarURL{};
	ENativeFriendStatus Status = ENativeFriendStatus::Unknown;
	FString ReasonCannotBeInvited{};
};

DECLARE_DELEGATE_ThreeParams(FOnGetNativeFriendListComplete, bool /*bWasSuccessful*/, TArray<UNativeFriendData*> /*Friends*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnSyncNativePlatformFriendListComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
