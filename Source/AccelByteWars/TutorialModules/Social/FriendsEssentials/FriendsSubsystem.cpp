// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Friends Interface and make sure it's valid.
    FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
    if (!ensure(FriendsInterface.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }

    // Grab prompt subsystem.
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);
}

void UFriendsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    // Clear on friends changed delegate.
    for (auto& DelegateHandle : OnFriendsChangeDelegateHandles)
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(DelegateHandle.Key, DelegateHandle.Value);
    }
}

// @@@SNIPSTART FriendsSubsystem.cpp-GetUniqueNetIdFromPlayerController
FUniqueNetIdPtr UFriendsSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
{
    if (!PC)
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-GetLocalUserNumFromPlayerController
int32 UFriendsSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
{
    int32 LocalUserNum = 0;

    if (!PC)
    {
        return LocalUserNum;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (LocalPlayer)
    {
        LocalUserNum = LocalPlayer->GetControllerId();
    }

    return LocalUserNum;
}
// @@@SNIPEND

#pragma region Module.8a Function Definitions

// @@@SNIPSTART FriendsSubsystem.cpp-GetCacheFriendList
void UFriendsSubsystem::GetCacheFriendList(const int32 LocalUserNum, bool bQueryUserInfo, const FOnGetCacheFriendListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot cache friend list. Friends Interface is not valid."));
        return;
    }
    
    // Try to get cached friend list first.
    TArray<TSharedRef<FOnlineFriend>> CachedFriendList;
    if (FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), CachedFriendList))
    {
        // Then, update the cached friends' information by querying their user information.
        TPartyMemberArray FriendIds;
        for (const TSharedRef<FOnlineFriend>& CachedFriend : CachedFriendList)
        {
            FriendIds.Add(CachedFriend.Get().GetUserId());
        }

        // Query friends' user information.
        if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
        {
            if(bQueryUserInfo)
            {
                StartupSubsystem->QueryUserInfo(
                    LocalUserNum,
                    FriendIds,
                    FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, OnComplete, LocalUserNum](
                        const FOnlineError& Error,
                        const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
                    {
                        // Refresh friends data with queried friend's user information.
                        TArray<TSharedRef<FOnlineFriend>> NewCachedFriendList{};
                        FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), NewCachedFriendList);
                        OnComplete.ExecuteIfBound(true, NewCachedFriendList, TEXT(""));
                    }));
            }
            else
            {
                OnComplete.ExecuteIfBound(true, CachedFriendList, TEXT(""));
            }
        }
    }
    // If none, request to backend then get the cached the friend list.
    else
    {
        FriendsInterface->ReadFriendsList(
            LocalUserNum, 
            TEXT(""), 
            FOnReadFriendsListComplete::CreateWeakLambda(this, [this, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& Error)
            {
                TArray<TSharedRef<FOnlineFriend>> CachedFriendList;
                FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), CachedFriendList);

                OnComplete.ExecuteIfBound(bWasSuccessful, CachedFriendList, Error);
            }
        ));
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-GetSelfFriendCode
void UFriendsSubsystem::GetSelfFriendCode(const APlayerController* PC, const FOnGetSelfFriendCodeComplete& OnComplete)
{
    if (!ensure(UserInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot get self friend code. User Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    const FUniqueNetIdPtr LocalPlayerId = GetUniqueNetIdFromPlayerController(PC);
    if (!ensure(LocalPlayerId.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot get self friend code. LocalPlayer NetId is not valid."));
        return;
    }

    // Try to get friend code from cache.
    if (const TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(LocalUserNum, LocalPlayerId.ToSharedRef().Get()))
    {
        if (const TSharedPtr<FUserOnlineAccountAccelByte> UserAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserInfo))
        {
            const FString FriendCode = UserAccount->GetPublicCode();
            UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Successfully obtained self friend code: %s"), *FriendCode);
            OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(UserInfo.ToSharedRef(), this), FriendCode);
            return;
        }
    }

    // If not available on cache then query the user info to get friend code.
    if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
    {
        StartupSubsystem->QueryUserInfo(
            LocalUserNum,
            TPartyMemberArray{ LocalPlayerId.ToSharedRef() },
            FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, OnComplete](
                const FOnlineError& Error,
                const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
            {
                if (!Error.bSucceeded || UsersInfo.IsEmpty())
                {
                    UE_LOG_FRIENDS_ESSENTIALS(
                        Warning, TEXT("Failed to get self friend code: User info query has failed."));
                    OnComplete.ExecuteIfBound(false, nullptr, TEXT(""));
                    return;
                }

                const FString FriendCode = UsersInfo[0]->GetPublicCode();
                UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Successfully obtained self friend code: %s"), *FriendCode);
                OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(UsersInfo[0].ToSharedRef(), this), FriendCode);
            }));
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-FindFriend
void UFriendsSubsystem::FindFriend(const APlayerController* PC, const FString& InKeyword, const FOnFindFriendComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(UserInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot find a friend. Friends Interface or User Interface is not valid."));
        return;
    }
    
    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    const FUniqueNetIdPtr LocalPlayerId = GetUniqueNetIdFromPlayerController(PC);
    if (!ensure(LocalPlayerId.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot find friends. LocalPlayer NetId is not valid."));
        return;
    }

    GetCacheFriendList(LocalUserNum, true, FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, LocalPlayerId, LocalUserNum, InKeyword, OnComplete](bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            // Find friend by exact display name.
            UserInterface->QueryUserIdMapping(LocalPlayerId.ToSharedRef().Get(), InKeyword, IOnlineUser::FOnQueryUserMappingComplete::CreateUObject(this, &ThisClass::OnFindFriendComplete, LocalUserNum, OnComplete));
        }
        else
        {
            OnComplete.ExecuteIfBound(false, nullptr, ErrorMessage);
        }
    }));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-OnFindFriendComplete
void UFriendsSubsystem::OnFindFriendComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayName, const FUniqueNetId& FoundUserId, const FString& Error, int32 LocalUserNum, const FOnFindFriendComplete OnComplete)
{
    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to find a friend with keyword: %s"), *DisplayName);

        // Check if the found user is the player it self.
        if (UserId == FoundUserId) 
        {
            OnComplete.ExecuteIfBound(false, nullptr, CANNOT_INVITE_FRIEND_SELF.ToString());
            return;
        }

        // Check if the found user is already friend.
        TSharedPtr<FOnlineFriend> FoundFriend = FriendsInterface->GetFriend(LocalUserNum, FoundUserId, TEXT(""));
        if (FoundFriend.IsValid())
        {
            OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(FoundFriend.ToSharedRef(), this), TEXT(""));
            return;
        }

        // Request the found user information to backend (to retrieve avatar URL, display name, etc.)
        if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
        {
            StartupSubsystem->QueryUserInfo(
                LocalUserNum,
                TPartyMemberArray{ FoundUserId.AsShared() },
                FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, OnComplete](
                    const FOnlineError& Error,
                    const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
                {
                    if (Error.bSucceeded && !UsersInfo.IsEmpty())
                    {
                        OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(UsersInfo[0].ToSharedRef(), this), TEXT(""));
                    }
                    else
                    {
                        OnComplete.ExecuteIfBound(false, nullptr, Error.ErrorMessage.ToString());
                    }
                }));
        }
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to find a friend with keyword: %s"), *DisplayName);
        OnComplete.ExecuteIfBound(false, nullptr, Error);
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-SendFriendRequestById
void UFriendsSubsystem::SendFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnSendFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot send friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(SEND_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Send friend requests by friend's user ID. We leave the ListName argument empty since the AccelByte OSS does not require it.
    FriendsInterface->SendInvite(LocalUserNum, *FriendUserId.GetUniqueNetId().Get(), TEXT(""), FOnSendInviteComplete::CreateUObject(this, &ThisClass::OnSendFriendRequestComplete, OnComplete));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-SendFriendRequestByCode
void UFriendsSubsystem::SendFriendRequest(const APlayerController* PC, const FString& FriendCode, const FOnSendFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot send friend request by friend code. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Send friend requests by friend code. We leave the ListName argument empty since the AccelByte OSS does not require it.
    FriendsInterface->SendInvite(LocalUserNum, FriendCode, TEXT(""), FOnSendInviteComplete::CreateUObject(this, &ThisClass::OnSendFriendRequestByFriendCodeComplete, OnComplete));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-OnSendFriendRequestComplete
void UFriendsSubsystem::OnSendFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    TSharedPtr<FOnlineFriend> FoundFriend = FriendsInterface->GetFriend(LocalUserNum, FriendId, TEXT(""));
    if (bWasSuccessful && FoundFriend.IsValid())
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to send a friend request."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_SEND_FRIEND_REQUEST);
        OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(FoundFriend.ToSharedRef(), this), TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to send a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, nullptr, ErrorStr);
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-OnSendFriendRequestByFriendCodeComplete
void UFriendsSubsystem::OnSendFriendRequestByFriendCodeComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete)
{
    // Get cached friend info to retrieve the updated friend data to be returned to the callback.
    FUniqueNetIdPtr FriendUserId = FriendId.AsShared();
    GetCacheFriendList(
        LocalUserNum,
        true,
        FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, LocalUserNum, bWasSuccessful, FriendUserId, ErrorStr, OnComplete]
        (bool bQueryWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
        {
            UFriendData* FriendData = nullptr;
            if (FriendUserId) 
            {
                TSharedPtr<FOnlineFriend> FoundFriend = FriendsInterface->GetFriend(LocalUserNum, FriendUserId.ToSharedRef().Get(), TEXT(""));
                if (FoundFriend)
                {
                    FriendData = UFriendData::ConvertToFriendData(FoundFriend.ToSharedRef(), this);
                }
            }

            if (bWasSuccessful)
            {
                UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to send a friend request by friend code."));

                PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_SEND_FRIEND_REQUEST_BY_FRIEND_CODE);
                OnComplete.ExecuteIfBound(true, FriendData, TEXT(""));
            }
            else
            {
                UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to send a friend request by friend code. Error: %s"), *ErrorStr);
                OnComplete.ExecuteIfBound(false, FriendData, ErrorStr);
            }
        }
    ));
}
// @@@SNIPEND

#pragma endregion


#pragma region Module.8b Function Definitions

// @@@SNIPSTART FriendsSubsystem.cpp-BindOnCachedFriendsDataUpdated
void UFriendsSubsystem::BindOnCachedFriendsDataUpdated(const APlayerController* PC, const FOnCachedFriendsDataUpdated& Delegate)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Add on friends changed delegate.
    OnFriendsChangeDelegateHandles.Add(LocalUserNum, FriendsInterface->AddOnFriendsChangeDelegate_Handle(LocalUserNum, FOnFriendsChangeDelegate::CreateWeakLambda(this, [Delegate]() { Delegate.ExecuteIfBound(); })));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-UnbindOnCachedFriendsDataUpdated
void UFriendsSubsystem::UnbindOnCachedFriendsDataUpdated(const APlayerController* PC)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Clear on friends changed delegate.
    FDelegateHandle TempHandle = OnFriendsChangeDelegateHandles[LocalUserNum];
    if (TempHandle.IsValid())
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(LocalUserNum, TempHandle);
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-GetInboundFriendRequestList
void UFriendsSubsystem::GetInboundFriendRequestList(const APlayerController* PC, const FOnGetInboundFriendRequestListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot query friend request list. Friends Interface is not valid."));
        return;
    }

    // Get friend inbound request list from cache.
    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    GetCacheFriendList(LocalUserNum, true, FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, OnComplete](bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            // Filter pending inbound friend requests.
            CachedFriendList = CachedFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
            {
                return Friend->GetInviteStatus() == EInviteStatus::PendingInbound;
            });

            TArray<UFriendData*> InboundFriendRequestList;
            for (const TSharedRef<FOnlineFriend>& TempData : CachedFriendList)
            {
                InboundFriendRequestList.Add(UFriendData::ConvertToFriendData(TempData, this));
            }

            OnComplete.ExecuteIfBound(true, InboundFriendRequestList, TEXT(""));
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-GetOutboundFriendRequestList
void UFriendsSubsystem::GetOutboundFriendRequestList(const APlayerController* PC, const FOnGetOutboundFriendRequestListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot query friend request list. Friends Interface is not valid."));
        return;
    }

    // Get friend outbound request list from cache.
    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    GetCacheFriendList(LocalUserNum, true, FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, OnComplete](bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            // Filter pending outbound friend requests.
            CachedFriendList = CachedFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
            {
                return Friend->GetInviteStatus() == EInviteStatus::PendingOutbound;
            });

            TArray<UFriendData*> OutbondFriendRequestList;
            for (const TSharedRef<FOnlineFriend>& TempData : CachedFriendList)
            {
                OutbondFriendRequestList.Add(UFriendData::ConvertToFriendData(TempData, this));
            }

            OnComplete.ExecuteIfBound(true, OutbondFriendRequestList, TEXT(""));
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-AcceptFriendRequest
void UFriendsSubsystem::AcceptFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnAcceptFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot accept friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(ACCEPT_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    FriendsInterface->AcceptInvite(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""), FOnAcceptInviteComplete::CreateUObject(this, &ThisClass::OnAcceptFriendRequestComplete, OnComplete));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-OnAcceptFriendRequestComplete
void UFriendsSubsystem::OnAcceptFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnAcceptFriendRequestComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to accept a friend request."));
        
        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_ACCEPT_FRIEND_REQUEST);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to accept a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-RejectFriendRequest
void UFriendsSubsystem::RejectFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRejectFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot reject friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(REJECT_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    OnRejectFriendRequestCompleteDelegateHandle = FriendsInterface->AddOnRejectInviteCompleteDelegate_Handle(LocalUserNum, FOnRejectInviteCompleteDelegate::CreateUObject(this, &ThisClass::OnRejectFriendRequestComplete, OnComplete));
    FriendsInterface->RejectInvite(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-OnRejectFriendRequestComplete
void UFriendsSubsystem::OnRejectFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRejectFriendRequestComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnRejectInviteCompleteDelegate_Handle(LocalUserNum, OnRejectFriendRequestCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to reject a friend request."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_REJECT_FRIEND_REQUEST);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to reject a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}
// @@@SNIPEND

// @@@SNIPSTART FriendsSubsystem.cpp-CancelFriendRequest
void UFriendsSubsystem::CancelFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnCancelFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot cancel friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(CANCEL_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    OnCancelFriendRequestCompleteDelegateHandle = FriendsInterface->AddOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, FOnDeleteFriendCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelFriendRequestComplete, OnComplete));
    FriendsInterface->DeleteFriend(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}
// @@@SNIPEND

void UFriendsSubsystem::GetFriendsInviteStatus(const APlayerController* PC, TArray<UFriendData*> PlayerData, const FOnGetPlayersInviteStatusComplete& OnComplete)
{    
    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    GetCacheFriendList(LocalUserNum, false, FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, PlayerData, OnComplete](bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            for(UFriendData* Player : PlayerData)
            {
                const TSharedRef<FOnlineFriend>* FriendData = CachedFriendList.FindByPredicate([Player](TSharedRef<FOnlineFriend> Friend)
                {
                   return Player->UserId == Friend->GetUserId(); 
                });
            
                if(FriendData != nullptr)
                {
                    EInviteStatus::Type InviteStatus = FriendData->Get().GetInviteStatus();
                    switch (InviteStatus)
                    {
                    case EInviteStatus::Accepted:
                        Player->Status = EFriendStatus::Accepted;
                        Player->bCannotBeInvited = true;
                        Player->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Already friend", "Already friend").ToString();
                        break;
                    case EInviteStatus::PendingInbound:
                        Player->Status = EFriendStatus::PendingInbound;
                        Player->bCannotBeInvited = true;
                        Player->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "You've been invited", "You've been invited").ToString();
                        break;
                    case EInviteStatus::PendingOutbound:
                        Player->Status = EFriendStatus::PendingOutbound;
                        Player->bCannotBeInvited = true;
                        Player->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Already invited", "Already invited").ToString();
                        break;
                    case EInviteStatus::Blocked:
                        Player->Status = EFriendStatus::Blocked;
                        Player->bCannotBeInvited = true;
                        Player->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Blocked", "Blocked").ToString();
                        break;
                    default:
                        Player->Status = EFriendStatus::Unknown;
                        Player->bCannotBeInvited = false;
                    }
                }
            }

            OnComplete.ExecuteIfBound(true, TEXT(""));
        }
        else
        {
            OnComplete.ExecuteIfBound(false, ErrorMessage);
        }
    }));
}

// @@@SNIPSTART FriendsSubsystem.cpp-OnCancelFriendRequestComplete
void UFriendsSubsystem::OnCancelFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnCancelFriendRequestComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, OnCancelFriendRequestCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to cancel a friend request."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_CANCEL_FRIEND_REQUEST);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to cancel a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}
// @@@SNIPEND

#pragma endregion


#pragma region Module.8c Function Definitions

// @@@SNIPSTART FriendsSubsystem.cpp-GetFriendList
void UFriendsSubsystem::GetFriendList(const APlayerController* PC, const FOnGetFriendListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot get friend list. Friends Interface is not valid."));
        return;
    }

    // Get accepted friend list from cache.
    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    GetCacheFriendList(LocalUserNum, true, FOnGetCacheFriendListComplete::CreateWeakLambda(this, [this, OnComplete](bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>>& CachedFriendList, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            // Filter accepted friends.
            CachedFriendList = CachedFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
            {
                return Friend->GetInviteStatus() == EInviteStatus::Accepted;
            });

            TArray<UFriendData*> AcceptedFriendList;
            for (const TSharedRef<FOnlineFriend>& TempData : CachedFriendList)
            {
                AcceptedFriendList.Add(UFriendData::ConvertToFriendData(TempData, this));
            }

            OnComplete.ExecuteIfBound(true, AcceptedFriendList, TEXT(""));
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}
// @@@SNIPEND

#pragma endregion

TArray<UTutorialModuleSubsystem::FCheatCommandEntry> UFriendsSubsystem::GetCheatCommandEntries()
{
    TArray<FCheatCommandEntry> OutArray = {};

    // Show local user (index 0) friends
    OutArray.Add(FCheatCommandEntry(
        *CommandReadFriendList,
        TEXT("Print friend list"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::DisplayFriendList)));

    return OutArray;
}

void UFriendsSubsystem::DisplayFriendList(const TArray<FString>& Args)
{
    // Construct OnComplete to display the info
    FOnGetFriendListComplete OnGetFriendListComplete = FOnGetFriendListComplete::CreateWeakLambda(
        this, [this](bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage)
        {
            FString OutString = FString::Printf(LINE_TERMINATOR);
            if (!bWasSuccessful)
            {
                OutString = TEXT("Query failed");
            }
            else
            {
                if (Friends.IsEmpty())
                {
                    OutString = TEXT("Friend list is empty");
                }
                else
                {
                    for (const UFriendData* Friend : Friends)
                    {
                        OutString += FString::Printf(
                            TEXT("%s%s (%s)%s%sDisplay name: %s%sAvatar URL: %s"),
                            LINE_TERMINATOR,
                            *Friend->GetName(),
                            *Friend->UserId->ToDebugString(),
                            LINE_TERMINATOR,
                            *Friend->DisplayName,
                            LINE_TERMINATOR,
                            *Friend->AvatarURL,
                            LINE_TERMINATOR);
                    }
                }
            }

            GetWorld()->GetGameViewport()->ViewportConsole->OutputText(OutString);
        });

    GetFriendList(
        GetWorld()->GetFirstLocalPlayerFromController()->PlayerController,
        OnGetFriendListComplete);
}

#undef LOCTEXT_NAMESPACE