// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/ManagingFriendsSubsystem.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget_Starter.h"

void UManagingFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Friends Interface and make sure it's valid.
    FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
    if (!ensure(FriendsInterface.IsValid()))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }

    // Grab prompt subsystem.
    PromptSubsystem = GetGameInstance()->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);

    // Assign action button to unfriend.
    FTutorialModuleGeneratedWidget* UnfriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unfriend"), AssociateTutorialModule->GeneratedWidgets);
    ensure(UnfriendButtonMetadata);
    UnfriendButtonMetadata->ButtonAction.BindWeakLambda(this, [this]()
    {
        UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
        if (!ParentWidget)
        {
            return;
        }

        FUniqueNetIdRepl FriendUserId = nullptr;
        if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
        {
            ensure(FriendDetailsWidget->GetCachedFriendData());
            FriendUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
        }
        else if (const UFriendDetailsWidget_Starter* FriendDetailsWidget_Starter = Cast<UFriendDetailsWidget_Starter>(ParentWidget))
        {
            ensure(FriendDetailsWidget_Starter->GetCachedFriendData());
            FriendUserId = FriendDetailsWidget_Starter->GetCachedFriendData()->UserId;
        }

        ensure(FriendUserId.IsValid());

        OnUnfriendButtonClicked(
            ParentWidget->GetOwningPlayer(),
            FriendUserId,
            FOnUnfriendComplete::CreateWeakLambda(this, [ParentWidget](bool bWasSuccessful, const FString& ErrorMessage)
            {
                // Close the friend details widget if successful.
                if (bWasSuccessful && ParentWidget)
                {
                    ParentWidget->DeactivateWidget();
                }
            }
        ));
    });

    // Assign action button to block a player.
    FTutorialModuleGeneratedWidget* BlockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_block_player"), AssociateTutorialModule->GeneratedWidgets);
    ensure(BlockPlayerButtonMetadata);
    BlockPlayerButtonMetadata->ButtonAction.BindWeakLambda(this, [this]()
    {
        UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
        if (!ParentWidget)
        {
            return;
        }

        FUniqueNetIdRepl BlockedPlayerUserId = nullptr;
        if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
        {
            ensure(FriendDetailsWidget->GetCachedFriendData());
            BlockedPlayerUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
        }
        else if (const UFriendDetailsWidget_Starter* FriendDetailsWidget_Starter = Cast<UFriendDetailsWidget_Starter>(ParentWidget))
        {
            ensure(FriendDetailsWidget_Starter->GetCachedFriendData());
            BlockedPlayerUserId = FriendDetailsWidget_Starter->GetCachedFriendData()->UserId;
        }

        ensure(BlockedPlayerUserId.IsValid());

        OnBlockButtonClicked(
            ParentWidget->GetOwningPlayer(),
            BlockedPlayerUserId,
            FOnBlockPlayerComplete::CreateWeakLambda(this, [ParentWidget](bool bWasSuccessful, const FString& ErrorMessage)
            {
                // Close the friend details widget if successful.
                if (bWasSuccessful && ParentWidget)
                {
                    ParentWidget->DeactivateWidget();
                }
            }
        ));
    });
}

void UManagingFriendsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    // Clear on blocked players changed delegate.
    for (auto& DelegateHandle : OnBlockedPlayersChangeDelegateHandles)
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(DelegateHandle.Key, DelegateHandle.Value);
    }
}

FUniqueNetIdPtr UManagingFriendsSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
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

int32 UManagingFriendsSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
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


#pragma region Module.12 General Function Definitions

void UManagingFriendsSubsystem::OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete)
{
    Unfriend(PC, FriendUserId, OnComplete);
}

void UManagingFriendsSubsystem::OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete)
{
    BlockPlayer(PC, BlockedPlayerUserId, OnComplete);
}

#pragma endregion


#pragma region Module.12 Function Definitions

void UManagingFriendsSubsystem::BindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC, const FOnGetCacheBlockedPlayersDataUpdated& Delegate)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Add on blocked players changed delegate.
    OnBlockedPlayersChangeDelegateHandles.Add(LocalUserNum, FriendsInterface->AddOnBlockListChangeDelegate_Handle(LocalUserNum, FOnBlockListChangeDelegate::CreateWeakLambda(this, [Delegate](int32, const FString&) { Delegate.ExecuteIfBound(); })));
}

void UManagingFriendsSubsystem::UnbindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC)
{
    ensure(FriendsInterface);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Clear on blocked players changed delegate.
    FDelegateHandle TempHandle = OnBlockedPlayersChangeDelegateHandles[LocalUserNum];
    if (TempHandle.IsValid())
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(LocalUserNum, TempHandle);
    }
}

void UManagingFriendsSubsystem::GetBlockedPlayerList(const APlayerController* PC, const FOnGetBlockedPlayerListComplete& OnComplete)
{
    if (!ensure(FriendsInterface))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot cache blocked player list. Friends Interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    const FUniqueNetIdPtr PlayerNetId = GetUniqueNetIdFromPlayerController(PC);

    // Try to get cached blocked player list first.
    TArray<TSharedRef<FOnlineBlockedPlayer>> CachedBlockedPlayerList;
    if (FriendsInterface->GetBlockedPlayers(PlayerNetId->AsShared().Get(), CachedBlockedPlayerList))
    {
        // Then, update the cached blocked players' information by querying their user information.
        TPartyMemberArray BlockedPlayerIds;
        for (const TSharedRef<FOnlineBlockedPlayer>& CachedBlockedPlayer : CachedBlockedPlayerList)
        {
            BlockedPlayerIds.Add(CachedBlockedPlayer.Get().GetUserId());
        }

        // Create callback to handle queried blocked players' user information.
        OnQueryUserInfoCompleteDelegateHandle = UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(
            LocalUserNum,
            FOnQueryUserInfoCompleteDelegate::CreateWeakLambda(this, [this, PlayerNetId, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& Error)
            {
                UserInterface->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegateHandle);

                // Refresh blocked players data with queried blocked players' user information.
                TArray<TSharedRef<FOnlineBlockedPlayer>> NewCachedBlockedPlayerList;
                FriendsInterface->GetBlockedPlayers(PlayerNetId->AsShared().Get(), NewCachedBlockedPlayerList);
                for (const TSharedRef<FOnlineBlockedPlayer>& NewCachedBlockedPlayer : NewCachedBlockedPlayerList)
                {
                    // Update blocked player's avatar URL based on queried friend's user information.
                    FString UserAvatarURL;
                    TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(LocalUserNum, NewCachedBlockedPlayer.Get().GetUserId().Get());
                    UserInfo->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, UserAvatarURL);
                    StaticCastSharedRef<FOnlineBlockedPlayerAccelByte>(NewCachedBlockedPlayer).Get().SetUserLocalAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, UserAvatarURL);
                }

                // Return blocked players to the callback.
                TArray<UFriendData*> BlockedPlayers;
                for (const TSharedRef<FOnlineBlockedPlayer>& TempData : NewCachedBlockedPlayerList)
                {
                    BlockedPlayers.Add(UFriendData::ConvertToFriendData(TempData));
                }

                OnComplete.ExecuteIfBound(true, BlockedPlayers, TEXT(""));
            }
        ));

        // Query blocked players' user information.
        UserInterface->QueryUserInfo(LocalUserNum, BlockedPlayerIds);
    }
    // If none, request to backend then get the cached the blocked player list.
    else
    {
        OnQueryBlockedPlayersCompleteDelegateHandle = FriendsInterface->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
            FOnQueryBlockedPlayersCompleteDelegate::CreateWeakLambda(this, [this, OnComplete](const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error)
            {
                if (!bWasSuccessful)
                {
                    OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), Error);
                    return;
                }

                TArray<TSharedRef<FOnlineBlockedPlayer>> CachedBlockedPlayer;
                FriendsInterface->GetBlockedPlayers(UserId, CachedBlockedPlayer);

                // Return blocked players to the callback.
                TArray<UFriendData*> BlockedPlayers;
                for (const TSharedRef<FOnlineBlockedPlayer>& TempData : CachedBlockedPlayer)
                {
                    BlockedPlayers.Add(UFriendData::ConvertToFriendData(TempData));
                }

                OnComplete.ExecuteIfBound(true, BlockedPlayers, TEXT(""));
            }
        ));

        FriendsInterface->QueryBlockedPlayers(PlayerNetId->AsShared().Get());
    }
}

void UManagingFriendsSubsystem::Unfriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot unfriend a friend. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(UNFRIEND_FRIEND_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    OnUnfriendCompleteDelegateHandle = FriendsInterface->AddOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, FOnDeleteFriendCompleteDelegate::CreateUObject(this, &ThisClass::OnUnfriendComplete, OnComplete));
    FriendsInterface->DeleteFriend(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}

void UManagingFriendsSubsystem::OnUnfriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnUnfriendComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, OnUnfriendCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to unfriend a friend."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_UNFRIEND_FRIEND);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to unfriend a friend. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}

void UManagingFriendsSubsystem::BlockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot block a player. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(BLOCK_PLAYER_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    OnBlockPlayerCompleteDelegateHandle = FriendsInterface->AddOnBlockedPlayerCompleteDelegate_Handle(LocalUserNum, FOnBlockedPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnBlockPlayerComplete, OnComplete));
    FriendsInterface->BlockPlayer(LocalUserNum, BlockedPlayerUserId.GetUniqueNetId().ToSharedRef().Get());
}

void UManagingFriendsSubsystem::OnBlockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnBlockPlayerComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnBlockedPlayerCompleteDelegate_Handle(LocalUserNum, OnBlockPlayerCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to block a player."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_BLOCK_PLAYER);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to block a player. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}

void UManagingFriendsSubsystem::UnblockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete)
{
    if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot unblock a player. Friends Interface or Prompt Subsystem is not valid."));
        return;
    }

    PromptSubsystem->ShowLoading(UNBLOCK_PLAYER_MESSAGE);

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
    OnUnblockPlayerCompleteDelegateHandle = FriendsInterface->AddOnUnblockedPlayerCompleteDelegate_Handle(LocalUserNum, FOnBlockedPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnUnblockPlayerComplete, OnComplete));
    FriendsInterface->UnblockPlayer(LocalUserNum, BlockedPlayerUserId.GetUniqueNetId().ToSharedRef().Get());
}

void UManagingFriendsSubsystem::OnUnblockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnUnblockPlayerComplete OnComplete)
{
    PromptSubsystem->HideLoading();

    FriendsInterface->ClearOnUnblockedPlayerCompleteDelegate_Handle(LocalUserNum, OnUnblockPlayerCompleteDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to unblock a player."));

        PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_UNBLOCK_PLAYER);
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to unblock a player. Error: %s"), *ErrorStr);

        PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
        OnComplete.ExecuteIfBound(false, ErrorStr);
    }
}

#pragma endregion