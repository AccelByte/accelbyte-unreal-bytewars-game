// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "FriendsEssentialsModels.generated.h"

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

UCLASS()
class ACCELBYTEWARS_API UFriendData : public UObject
{
	GENERATED_BODY()

public:
	UFriendData() : bIsOnline(false), bCannotBeInvited(false) {}

	FUniqueNetIdPtr UserId;
	FString DisplayName;
	FString AvatarURL;
	EFriendStatus Status = EFriendStatus::Unknown;

    bool bIsOnline;
    FDateTime LastOnline;

	bool bCannotBeInvited;
	FString ReasonCannotBeInvited;

    static UFriendData* ConvertToFriendData(TSharedRef<FOnlineUser> OnlineUser)
    {
        UFriendData* FriendData = NewObject<UFriendData>();

        FriendData->UserId = OnlineUser->GetUserId();
        FriendData->DisplayName = OnlineUser->GetDisplayName();
        OnlineUser->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendData->AvatarURL);
        FriendData->Status = EFriendStatus::Unknown;
        FriendData->bCannotBeInvited = false;

        return FriendData;
    }

    static UFriendData* ConvertToFriendData(TSharedRef<FOnlineFriend> OnlineUser)
    {
        UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser));

        switch (OnlineUser->GetInviteStatus())
        {
        case EInviteStatus::Accepted:
            FriendData->Status = EFriendStatus::Accepted;
            FriendData->bCannotBeInvited = true;
            FriendData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Already friend", "Already friend").ToString();
            break;
        case EInviteStatus::PendingInbound:
            FriendData->Status = EFriendStatus::PendingInbound;
            FriendData->bCannotBeInvited = true;
            FriendData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "You've been invited", "You've been invited").ToString();
            break;
        case EInviteStatus::PendingOutbound:
            FriendData->Status = EFriendStatus::PendingOutbound;
            FriendData->bCannotBeInvited = true;
            FriendData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Already invited", "Already invited").ToString();
            break;
        case EInviteStatus::Blocked:
            FriendData->Status = EFriendStatus::Blocked;
            FriendData->bCannotBeInvited = true;
            FriendData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Blocked", "Blocked").ToString();
            break;
        default:
            FriendData->Status = EFriendStatus::Unknown;
            FriendData->bCannotBeInvited = false;
        }

        return FriendData;
    }

    static UFriendData* ConvertToFriendData(TSharedRef<FOnlineBlockedPlayer> OnlineUser)
    {
        UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser));

        FriendData->Status = EFriendStatus::Blocked;
        FriendData->bCannotBeInvited = true;
        FriendData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Blocked", "Blocked").ToString();

        return FriendData;
    }
};

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

DECLARE_DELEGATE_ThreeParams(FOnGetCacheFriendListComplete, bool /*bWasSuccessful*/, TArray<TSharedRef<FOnlineFriend>>& /*CachedFriendList*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE(FOnCachedFriendsDataUpdated);

DECLARE_DELEGATE_ThreeParams(FOnGetSelfFriendCodeComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*FriendCode*/);
DECLARE_DELEGATE_ThreeParams(FOnFindFriendComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetInboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetOutboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetFriendListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*Friends*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_ThreeParams(FOnSendFriendRequestComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnAcceptFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnRejectFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnCancelFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_ThreeParams(FOnGetBlockedPlayerListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*BlockedPlayers*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE(FOnGetCacheBlockedPlayersDataUpdated);

DECLARE_DELEGATE_TwoParams(FOnUnfriendComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnBlockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnUnblockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);