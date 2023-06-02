// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/FriendsEssentialsSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
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

void UFriendsEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FUniqueNetIdPtr UFriendsEssentialsSubsystem::GetPlayerUniqueNetId(const APlayerController* PC) const
{
    if (!ensure(PC))
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!ensure(LocalPlayer))
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

int32 UFriendsEssentialsSubsystem::GetPlayerControllerId(const APlayerController* PC) const
{
    int32 LocalUserNum = 0;

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (LocalPlayer)
    {
        LocalUserNum = LocalPlayer->GetControllerId();
    }

    return LocalUserNum;
}


#pragma region Module.8a Function Definitions

void UFriendsEssentialsSubsystem::CacheFriendList(const APlayerController* PC, const FOnCacheFriendsDataComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot cache friend list. Friends Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetPlayerControllerId(PC);

    FriendsInterface->ReadFriendsList(LocalUserNum, TEXT(""), FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnCacheFriendListComplete, OnComplete));
}

void UFriendsEssentialsSubsystem::OnCacheFriendListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& Error, const FOnCacheFriendsDataComplete OnComplete)
{
    OnComplete.ExecuteIfBound(bWasSuccessful, Error);
}

void UFriendsEssentialsSubsystem::FindFriend(const APlayerController* PC, const FString& InKeyword, const FOnFindFriendComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(UserInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot find a friend. Friends Interface or User Interface is not valid."));
        return;
    }
    
    const int32 LocalUserNum = GetPlayerControllerId(PC);
    const FUniqueNetIdPtr LocalPlayerId = GetPlayerUniqueNetId(PC);
    if (!ensure(LocalPlayerId.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot find friends. LocalPlayer NetId is not valid."));
        return;
    }

    // Before request find friend to backend, check if friend list is queried in cache.
    // This is necessary to check the found user status (e.g. is already friend, invitation request already sent).
    TArray<TSharedRef<FOnlineFriend>> OutFriendList;
    if (!FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), OutFriendList))
    {
        CacheFriendList(PC, FOnCacheFriendsDataComplete::CreateWeakLambda(this, [this, PC, InKeyword, OnComplete](bool bWasSuccessful, const FString& ErrorMessage)
        {
            if (bWasSuccessful)
            {
                FindFriend(PC, InKeyword, OnComplete);
            }
            else
            {
                OnComplete.ExecuteIfBound(false, nullptr, ErrorMessage);
            }
        }));
        return;
    }

    // Find a friend.
    UserInterface->QueryUserIdMapping(LocalPlayerId.ToSharedRef().Get(), InKeyword, IOnlineUser::FOnQueryUserMappingComplete::CreateUObject(this, &ThisClass::OnFindFriendComplete, LocalUserNum, OnComplete));
}

void UFriendsEssentialsSubsystem::OnFindFriendComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Username, const FUniqueNetId& FoundUserId, const FString& Error, int32 LocalUserNum, const FOnFindFriendComplete OnComplete)
{
    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to find a friend with keyword: %s"), *Username);

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
            OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(FoundFriend.ToSharedRef()), TEXT(""));
            return;
        }

        // Request the found user information to backend (to retrieve avatar URL, username, etc.)
        OnQueryUserInfoCompleteDelegateHandle = UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(
            LocalUserNum,
            FOnQueryUserInfoCompleteDelegate::CreateWeakLambda(this, [this, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorStr)
            {
                UserInterface->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegateHandle);

                if (bWasSuccessful)
                {
                    OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(UserInterface->GetUserInfo(LocalUserNum, UserIds[0].Get()).ToSharedRef()), TEXT(""));
                }
                else
                {
                    OnComplete.ExecuteIfBound(false, nullptr, ErrorStr);
                }
            }
        ));
        UserInterface->QueryUserInfo(LocalUserNum, TPartyMemberArray{ FoundUserId.AsShared() });
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to find a friend with keyword: %s"), *Username);
        OnComplete.ExecuteIfBound(false, nullptr, Error);
    }
}

void UFriendsEssentialsSubsystem::SendFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnSendFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot send friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(SEND_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetPlayerControllerId(PC);
    FriendsInterface->SendInvite(LocalUserNum, *FriendUserId.GetUniqueNetId().Get(), TEXT(""), FOnSendInviteComplete::CreateUObject(this, &ThisClass::OnSendFriendRequestComplete, OnComplete));
}

void UFriendsEssentialsSubsystem::OnSendFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnSendFriendRequestComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    TSharedPtr<FOnlineFriend> FoundFriend = FriendsInterface->GetFriend(LocalUserNum, FriendId, TEXT(""));
    if (bWasSuccessful && FoundFriend.IsValid())
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to send a friend request."));
        
        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_SEND_FRIEND_REQUEST);
        OnComplete.ExecuteIfBound(true, UFriendData::ConvertToFriendData(FoundFriend.ToSharedRef()), TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to send a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, nullptr, ErrorStr);
    }
}

#pragma endregion


#pragma region Module.8b Function Definitions

void UFriendsEssentialsSubsystem::BindOnCachedFriendsDataUpdated(const APlayerController* PC, const FOnFriendsChangeDelegate& Delegate)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetPlayerControllerId(PC);

    // Add on friends changed delegate.
    OnFriendsChangeDelegateHandles.Add(LocalUserNum, FriendsInterface->AddOnFriendsChangeDelegate_Handle(LocalUserNum, FOnFriendsChangeDelegate::CreateWeakLambda(this, [Delegate]() { Delegate.ExecuteIfBound(); })));
}

void UFriendsEssentialsSubsystem::UnbindOnCachedFriendsDataUpdated(const APlayerController* PC)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetPlayerControllerId(PC);

    // Clear on friends changed delegate.
    FDelegateHandle TempHandle = OnFriendsChangeDelegateHandles[LocalUserNum];
    if (TempHandle.IsValid())
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(LocalUserNum, TempHandle);
    }
}

void UFriendsEssentialsSubsystem::GetInboundFriendRequestList(const APlayerController* PC, const FOnGetInboundFriendRequestListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot query friend request list. Friends Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetPlayerControllerId(PC);

    // Try to get friend inbound request list from cache first.
    TArray<TSharedRef<FOnlineFriend>> OutFriendList;
    TArray<UFriendData*> FriendRequestList;
    if (FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), OutFriendList))
    {
        // Filter pending inbound friend requests.
        OutFriendList = OutFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
        {
            return Friend->GetInviteStatus() == EInviteStatus::PendingInbound;
        });
        
        for (const TSharedRef<FOnlineFriend> TempData : OutFriendList)
        {
            FriendRequestList.Add(UFriendData::ConvertToFriendData(TempData));
        }

        OnComplete.ExecuteIfBound(true, FriendRequestList, TEXT(""));
        return;
    }
    
    // If none, request to query from backend. Then, try the get the list again.
    CacheFriendList(PC, FOnCacheFriendsDataComplete::CreateWeakLambda(this, [this, PC, OnComplete](bool bWasSuccessful, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            GetInboundFriendRequestList(PC, OnComplete);
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}

void UFriendsEssentialsSubsystem::GetOutboundFriendRequestList(const APlayerController* PC, const FOnGetOutboundFriendRequestListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot query friend request list. Friends Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetPlayerControllerId(PC);

    // Try to get friend outbound request list from cache first.
    TArray<TSharedRef<FOnlineFriend>> OutFriendList;
    TArray<UFriendData*> FriendRequestList;
    if (FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), OutFriendList))
    {
        // Filter pending outbound friend requests.
        OutFriendList = OutFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
        {
            return Friend->GetInviteStatus() == EInviteStatus::PendingOutbound;
        });

        for (const TSharedRef<FOnlineFriend> TempData : OutFriendList)
        {
            FriendRequestList.Add(UFriendData::ConvertToFriendData(TempData));
        }

        OnComplete.ExecuteIfBound(true, FriendRequestList, TEXT(""));
        return;
    }

    // If none, request to query from backend. Then, try the get the list again.
    CacheFriendList(PC, FOnCacheFriendsDataComplete::CreateWeakLambda(this, [this, PC, OnComplete](bool bWasSuccessful, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            GetOutboundFriendRequestList(PC, OnComplete);
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}

void UFriendsEssentialsSubsystem::AcceptFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnAcceptFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot accept friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(ACCEPT_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetPlayerControllerId(PC);
    FriendsInterface->AcceptInvite(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""), FOnAcceptInviteComplete::CreateUObject(this, &ThisClass::OnAcceptFriendRequestComplete, OnComplete));
}

void UFriendsEssentialsSubsystem::OnAcceptFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnAcceptFriendRequestComplete OnComplete)
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

void UFriendsEssentialsSubsystem::RejectFriendRequest(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRejectFriendRequestComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot reject friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(REJECT_FRIEND_REQUEST_MESSAGE);

    const int32 LocalUserNum = GetPlayerControllerId(PC);
    OnRejectFriendRequestCompleteDelegateHandle = FriendsInterface->AddOnRejectInviteCompleteDelegate_Handle(LocalUserNum, FOnRejectInviteCompleteDelegate::CreateUObject(this, &ThisClass::OnRejectFriendRequestComplete, OnComplete));
    FriendsInterface->RejectInvite(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}

void UFriendsEssentialsSubsystem::OnRejectFriendRequestComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRejectFriendRequestComplete OnComplete)
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

void UFriendsEssentialsSubsystem::RemoveFriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnRemoveFriendComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot remove friend to unfriend or cancel friend request. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(REMOVE_FRIEND_MESSAGE);

    const int32 LocalUserNum = GetPlayerControllerId(PC);
    OnRemoveFriendCompleteDelegateHandle = FriendsInterface->AddOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, FOnDeleteFriendCompleteDelegate::CreateUObject(this, &ThisClass::OnRemoveFriendComplete, OnComplete));
    FriendsInterface->DeleteFriend(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}

void UFriendsEssentialsSubsystem::OnRemoveFriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnRemoveFriendComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, OnRemoveFriendCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Success to remove friend to unfriend or to cancel a friend request."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_REMOVE_FRIEND);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to remove friend to unfriend or to cancel a friend request. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}

#pragma endregion


#pragma region Module.8c Function Definitions

void UFriendsEssentialsSubsystem::GetFriendList(const APlayerController* PC, const FOnGetFriendListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot get friend list. Friends Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetPlayerControllerId(PC);
    const FUniqueNetIdPtr LocalPlayerId = GetPlayerUniqueNetId(PC);
    if (!ensure(LocalPlayerId.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Cannot get friend list. LocalPlayer NetId is not valid."));
        return;
    }

    // Try to get friend list from cache first.
    TArray<TSharedRef<FOnlineFriend>> OutFriendList;
    TArray<UFriendData*> FriendList;
    if (FriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), OutFriendList))
    {
        // Filter accepted friends.
        OutFriendList = OutFriendList.FilterByPredicate([](const TSharedRef<FOnlineFriend>& Friend)
        {
            return Friend->GetInviteStatus() == EInviteStatus::Accepted;
        });

        TPartyMemberArray FriendIds;
        for (const TSharedRef<FOnlineFriend> TempData : OutFriendList)
        {
            FriendIds.Add(TempData->GetUserId());
            FriendList.Add(UFriendData::ConvertToFriendData(TempData));
        }

        OnComplete.ExecuteIfBound(true, FriendList, TEXT(""));
        return;
    }

    // If none, request to query from backend. Then, try the get the list again.
    CacheFriendList(PC, FOnCacheFriendsDataComplete::CreateWeakLambda(this, [this, PC, OnComplete](bool bWasSuccessful, const FString& ErrorMessage)
    {
        if (bWasSuccessful)
        {
            GetFriendList(PC, OnComplete);
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), ErrorMessage);
        }
    }));
}

#pragma endregion


#undef LOCTEXT_NAMESPACE