// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FriendsEssentialsModels.generated.h"

UENUM()
enum class ESearchFriendType : uint8
{
    ByUserId = 0,
    ByDisplayName
};

UENUM()
enum class EFriendStatus : uint8
{
	Accepted = 0,
	PendingInbound,
	PendingOutbound,
	Blocked,
    Searched,
	Unknown
};

UCLASS()
class ACCELBYTEWARS_API UFriendData : public UObject
{
	GENERATED_BODY()

public:
	UFriendData() : bIsOnline(false), bCannotBeInvited(false) {}

	FUniqueNetIdRepl UserId;
	FString Username;
	FString AvatarURL;
	EFriendStatus Status = EFriendStatus::Unknown;

    bool bIsOnline;
    FDateTime LastOnline;

	bool bCannotBeInvited;
	FString ReasonCannotBeInvited;

    FString GetPresence() const
    {
        // If friend is online, simply return online.
        if (bIsOnline) 
        {
            return NSLOCTEXT("AccelByteWars", "Online", "Online").ToString();
        }

        // Only check last online within a month.
        const FDateTime CurrentTime = FDateTime::UtcNow();
        if (CurrentTime.GetMonth() != LastOnline.GetMonth()) 
        {
            return NSLOCTEXT("AccelByteWars", "Last Online a Long Ago", "Last Online a Long Ago").ToString();
        }

        // Check last online in days.
        if (CurrentTime.GetDay() > LastOnline.GetDay()) 
        {
            const int32 Days = CurrentTime.GetDay() - LastOnline.GetDay();
            return FText::Format(NSLOCTEXT("AccelByteWars", "Last Online Day(s) Ago", "Last Online %d Day(s) Ago"), Days).ToString();
        }

        // Check last online in hours.
        if (CurrentTime.GetHour() > LastOnline.GetHour())
        {
            const int32 Hours = CurrentTime.GetHour() - LastOnline.GetHour();
            return FText::Format(NSLOCTEXT("AccelByteWars", "Last Online Hour(s) Ago", "Last Online %d Hour(s) Ago"), Hours).ToString();
        }

        // Check last online in minutes.
        if (CurrentTime.GetMinute() > LastOnline.GetMinute()) 
        {
            const int32 Minutes = CurrentTime.GetMinute() - LastOnline.GetMinute();
            return FText::Format(NSLOCTEXT("AccelByteWars", "Last Online Minute(s) Ago", "Last Online %d Minute(s) Ago"), Minutes).ToString();
        }
        else 
        {
            return NSLOCTEXT("AccelByteWars", "Last Online a While Ago", "Last Online a While Ago").ToString();
        }
    }

    static UFriendData* ConvertToFriendData(TSharedRef<FOnlineUser> OnlineUser)
    {
        UFriendData* FriendData = NewObject<UFriendData>();

        FriendData->UserId = OnlineUser->GetUserId();
        FriendData->Username = OnlineUser->GetDisplayName();
        OnlineUser->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendData->AvatarURL);
        FriendData->Status = EFriendStatus::Unknown;
        FriendData->bCannotBeInvited = false;

        return FriendData;
    }

    static UFriendData* ConvertToFriendData(TSharedRef<FOnlineFriend> OnlineUser)
    {
        UFriendData* FriendData = ConvertToFriendData(StaticCast<TSharedRef<FOnlineUser>>(OnlineUser));

        FriendData->bIsOnline = OnlineUser->GetPresence().bIsOnline;
        FriendData->LastOnline = OnlineUser->GetPresence().LastOnline;

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

#define DEFAULT_FIND_FRIEND_ERROR_MESSAGE NSLOCTEXT("AccelByteWars", "Friend Not Found", "Friend Not Found")

#define SEND_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Sending Friend Request", "Sending Friend Request")
#define ACCEPT_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Accepting Friend", "Accepting Friend")
#define REJECT_FRIEND_REQUEST_MESSAGE NSLOCTEXT("AccelByteWars", "Rejecting Friend", "Rejecting Friend")
#define REMOVE_FRIEND_MESSAGE NSLOCTEXT("AccelByteWars", "Removing Friend", "Removing Friend")
#define BLOCK_PLAYER_MESSAGE NSLOCTEXT("AccelByteWars", "Blocking Player", "Blocking Player")
#define UNBLOCK_PLAYER_MESSAGE NSLOCTEXT("AccelByteWars", "Unblocking Player", "Unblocking Player")

#define SUCCESS_SEND_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is sent", "Friend request is sent")
#define SUCCESS_ACCEPT_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is sent", "Friend request is sent")
#define SUCCESS_REJECT_FRIEND_REQUEST NSLOCTEXT("AccelByteWars", "Friend request is rejected", "Friend request is rejected")
#define SUCCESS_REMOVE_FRIEND NSLOCTEXT("AccelByteWars", "Friend is canceled and removed", "Friend is canceled and removed")
#define SUCCESS_BLOCK_PLAYER NSLOCTEXT("AccelByteWars", "Player is blocked", "Player is blocked")
#define SUCCESS_UNBLOCK_PLAYER NSLOCTEXT("AccelByteWars", "Player is unblock", "Player is unblock")
#define CANNOT_INVITE_FRIEND_SELF NSLOCTEXT("AccelByteWars", "Cannot friend with yourself", "Cannot friend with yourself")

DECLARE_DELEGATE_TwoParams(FOnCacheFriendsDataComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE(FOnCachedFriendsDataUpdated);

DECLARE_DELEGATE_ThreeParams(FOnFindFriendComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetInboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetOutboundFriendRequestListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*FriendRequests*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetFriendListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*Friends*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_ThreeParams(FOnGetBlockedPlayerListComplete, bool /*bWasSuccessful*/, TArray<UFriendData*> /*BlockedPlayers*/, const FString& /*ErrorMessage*/);

DECLARE_DELEGATE_ThreeParams(FOnSendFriendRequestComplete, bool /*bWasSuccessful*/, UFriendData* /*FriendData*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnAcceptFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnRejectFriendRequestComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnRemoveFriendComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnBlockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);
DECLARE_DELEGATE_TwoParams(FOnUnblockPlayerComplete, bool /*bWasSuccessful*/, const FString& /*ErrorMessage*/);