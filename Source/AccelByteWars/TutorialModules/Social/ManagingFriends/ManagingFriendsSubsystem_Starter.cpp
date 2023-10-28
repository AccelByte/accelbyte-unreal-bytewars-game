// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ManagingFriendsSubsystem_Starter.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget_Starter.h"

void UManagingFriendsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
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
    UnfriendButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
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
    BlockPlayerButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
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

void UManagingFriendsSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    // Clear on blocked players changed delegate.
    for (auto& DelegateHandle : OnBlockedPlayersChangeDelegateHandles)
    {
        FriendsInterface->ClearOnFriendsChangeDelegate_Handle(DelegateHandle.Key, DelegateHandle.Value);
    }
}

FUniqueNetIdPtr UManagingFriendsSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
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

int32 UManagingFriendsSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
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

void UManagingFriendsSubsystem_Starter::OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete)
{
    // TODO: Call unfriend request here.
}

void UManagingFriendsSubsystem_Starter::OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete)
{
    // TODO: Call block player request here.
}

#pragma endregion


#pragma region Module.12 Function Definitions

// TODO: Add your Module.12 function definitions here.

#pragma endregion