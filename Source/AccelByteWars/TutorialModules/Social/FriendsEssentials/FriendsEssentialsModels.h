// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "FriendsEssentialsModels.generated.h"

#define ALREADY_FRIEND_REASON_MESSAGE NSLOCTEXT("AccelByteWars", "Already friend", "Already friend")
#define BEEN_INVITED_REASON_MESSAGE NSLOCTEXT("AccelByteWars", "You've been invited", "You've been invited")
#define ALREADY_INVITED_REASON_MESSAGE NSLOCTEXT("AccelByteWars", "Already invited", "Already invited")
#define BLOCKED_REASON_MESSAGE NSLOCTEXT("AccelByteWars", "Blocked", "Blocked")

// @@@SNIPSTART FriendsEssentialsModels.h-FriendStatusEnum
UENUM()
enum class EFriendStatus : uint8
{
	Accepted = 0,
	PendingInbound,
	PendingOutbound,
	Searched,
	Blocked,
	Unknown
};
// @@@SNIPEND

UENUM()
enum class EDataSource : uint8
{
	User = 0,
	RecentPlayer,
	GameSessionPlayer
};

// @@@SNIPSTART FriendsEssentialsModels.h-FriendDataClass
UCLASS()
class ACCELBYTEWARS_API UFriendData : public UObject
{
	GENERATED_BODY()

public:
	UFriendData() : bIsOnline(false), bCannotBeInvited(false) {}

	FUniqueNetIdPtr UserId;
	FString DisplayName{};
	FString AvatarURL{};
	EFriendStatus Status = EFriendStatus::Unknown;

	bool bIsOnline = false;
	FDateTime LastOnline{};

	bool bCannotBeInvited = false;
	FString ReasonCannotBeInvited{};

	EDataSource DataSource = EDataSource::User;

	static UFriendData* ConvertToFriendData(TSharedRef<FOnlineUser> OnlineUser, const UObject* Context)
	{
		if (!Context || !Context->GetWorld()) 
		{
			return nullptr;
		}
		
		IOnlineSubsystem* Subsystem = Online::GetSubsystem(Context->GetWorld());
		if (!Subsystem) 
		{
			return nullptr;
		}

		IOnlineUserPtr UserInterface = Subsystem->GetUserInterface();
		if (!UserInterface)
		{
			return nullptr;
		}

		UFriendData* FriendData = NewObject<UFriendData>();
		FriendData->UserId = OnlineUser->GetUserId();
		FriendData->DisplayName = OnlineUser->GetDisplayName();
		OnlineUser->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendData->AvatarURL);
		FriendData->Status = EFriendStatus::Unknown;
		FriendData->bCannotBeInvited = false;

		// If avatar attribute from online user object is empty, try fetch it from the cache.
		TSharedPtr<FOnlineUser> CachedUser = UserInterface->GetUserInfo(0, OnlineUser->GetUserId().Get());
		if (FriendData->AvatarURL.IsEmpty() && CachedUser)
		{
			CachedUser->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendData->AvatarURL);
		}

		return FriendData;
	}

	static UFriendData* ConvertToFriendData(TSharedRef<FOnlineFriend> OnlineUser, const UObject* Context)
	{
		UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser), Context);

		switch (OnlineUser->GetInviteStatus())
		{
		case EInviteStatus::Accepted:
			FriendData->Status = EFriendStatus::Accepted;
			FriendData->bCannotBeInvited = true;
			FriendData->ReasonCannotBeInvited = ALREADY_FRIEND_REASON_MESSAGE.ToString();
			break;
		case EInviteStatus::PendingInbound:
			FriendData->Status = EFriendStatus::PendingInbound;
			FriendData->bCannotBeInvited = true;
			FriendData->ReasonCannotBeInvited = BEEN_INVITED_REASON_MESSAGE.ToString();
			break;
		case EInviteStatus::PendingOutbound:
			FriendData->Status = EFriendStatus::PendingOutbound;
			FriendData->bCannotBeInvited = true;
			FriendData->ReasonCannotBeInvited = ALREADY_INVITED_REASON_MESSAGE.ToString();
			break;
		case EInviteStatus::Blocked:
			FriendData->Status = EFriendStatus::Blocked;
			FriendData->bCannotBeInvited = true;
			FriendData->ReasonCannotBeInvited = BLOCKED_REASON_MESSAGE.ToString();
			break;
		default:
			FriendData->Status = EFriendStatus::Unknown;
			FriendData->bCannotBeInvited = false;
		}

		return FriendData;
	}

	static UFriendData* ConvertToFriendData(TSharedRef<FOnlineBlockedPlayer> OnlineUser, const UObject* Context)
	{
		UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser), Context);

		FriendData->Status = EFriendStatus::Blocked;
		FriendData->bCannotBeInvited = true;
		FriendData->ReasonCannotBeInvited = BLOCKED_REASON_MESSAGE.ToString();

		return FriendData;
	}
	
	static UFriendData* ConvertToFriendData(TSharedRef<const FAccelByteUserInfo> OnlineUser)
	{
		UFriendData* FriendData = NewObject<UFriendData>();

		FriendData->UserId = OnlineUser->Id;
		FriendData->DisplayName = OnlineUser->DisplayName;
		FriendData->AvatarURL = OnlineUser->GameAvatarUrl;
		FriendData->Status = EFriendStatus::Unknown;
		FriendData->bCannotBeInvited = false;
		FriendData->DataSource = EDataSource::GameSessionPlayer;

		return FriendData;
	}
	
	static UFriendData* ConvertToFriendData(TSharedRef<FOnlineRecentPlayer> OnlineUser, const UObject* Context)
	{
		UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser), Context);

		FriendData->DataSource = EDataSource::RecentPlayer;

		return FriendData;
	}
};
// @@@SNIPEND

#define SEND_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Sending Friend Request", "Sending Friend Request")

#define ACCEPT_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Accepting Friend", "Accepting Friend")
#define REJECT_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Rejecting Friend", "Rejecting Friend")
#define CANCEL_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Canceling Friend Request", "Canceling Friend Request")

#define UNFRIEND_FRIEND_MESSAGE NSLOCTEXT("AccelByteWars", "Unfriending", "Unfriending")
#define BLOCK_PLAYER_MESSAGE NSLOCTEXT("AccelByteWars", "Blocking Player", "Blocking Player")
#define UNBLOCK_PLAYER_MESSAGE NSLOCTEXT("AccelByteWars", "Unblocking Player", "Unblocking Player")

#define CANNOT_INVITE_FRIEND_SELF NSLOCTEXT("AccelByteWars", "Cannot friend with yourself", "Cannot friend with yourself")
#define SUCCESS_SEND_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is sent", "Friend request is sent")
#define SUCCESS_SEND_FRIEND_REQUEST_BY_FRIEND_CODE NSLOCTEXT("AccelByteWars", "Friend request by friend code is sent", "Friend request by friend code is sent")
#define SUCCESS_ACCEPT_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is accepted", "Friend request is accepted")
#define SUCCESS_REJECT_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is rejected", "Friend request is rejected")
#define SUCCESS_CANCEL_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is canceled", "Friend request is canceled")

#define SUCCESS_UNFRIEND_FRIEND NSLOCTEXT("AccelByteWars", "Friend is removed", "Friend is removed")
#define SUCCESS_BLOCK_PLAYER NSLOCTEXT("AccelByteWars", "Player is blocked", "Player is blocked")
#define SUCCESS_UNBLOCK_PLAYER NSLOCTEXT("AccelByteWars", "Player is unblocked", "Player is unblocked")

// @@@SNIPSTART FriendsEssentialsModels.h-delegatemacro
// @@@MULTISNIP FriendsCacheDelegate {"selectedLines": ["1-2"]}
// @@@MULTISNIP FindFriendsDelegate {"selectedLines": ["1", "4-6"]}
// @@@MULTISNIP GetFriendRequestsDelegate {"selectedLines": ["8-9"]}
// @@@MULTISNIP FriendRequestActionsDelegate {"selectedLines": ["13-15"]}
// @@@MULTISNIP GetFriendsDelegate {"selectedLines": ["10"]}
// @@@MULTISNIP BlockedPlayersCacheDelegate {"selectedLines": ["17-18"]}
// @@@MULTISNIP ManagingFriendActionsDelegate {"selectedLines": ["20-22"]}
DECLARE_DELEGATE_ThreeParams(FOnGetCacheFriendListComplete, bool /*bWasSuccessful*/, TArray<TSharedRef<FOnlineFriend>>& /*CachedFriendList*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE(FOnCachedFriendsDataUpdated);

DECLARE_DELEGATE_ThreeParams(FOnGetSelfFriendCodeComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*FriendCode*/);
DECLARE_DELEGATE_ThreeParams(FOnFindFriendComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnSendFriendRequestComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_ThreeParams(FOnGetInboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetOutboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetFriendListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*Friends*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnGetPlayersInviteStatusComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_TwoParams(FOnAcceptFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnRejectFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnCancelFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_ThreeParams(FOnGetBlockedPlayerListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*BlockedPlayers*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE(FOnGetCacheBlockedPlayersDataUpdated);

DECLARE_DELEGATE_TwoParams(FOnUnfriendComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnBlockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnUnblockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnInviteAsFriendPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
// @@@SNIPEND