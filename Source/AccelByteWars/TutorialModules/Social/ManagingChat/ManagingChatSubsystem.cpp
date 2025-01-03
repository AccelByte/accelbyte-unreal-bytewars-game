// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ManagingChatSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteApiClient.h"
#include "Api/AccelByteChatApi.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"

void UManagingChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Assign action button to mute chat.
    MuteChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_mutechat"));
    if (MuteChatButtonMetadata)
    {
        // Mute chat.
        MuteChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
        {
            MuteChat(GetCurrentDisplayedFriendId());
        });

        MuteChatButtonMetadata->OnWidgetGenerated.AddWeakLambda(this, [this]()
        {
            UpdateGeneratedWidgets();
        });
    }

    // Assign action button to unmute chat.
    UnmuteChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unmutechat"));
    if (UnmuteChatButtonMetadata)
    {
        // Unmute chat.
        UnmuteChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
        {
            UnmuteChat(GetCurrentDisplayedFriendId());
        });
    }
}

void UManagingChatSubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (MuteChatButtonMetadata) 
    {
        MuteChatButtonMetadata->ButtonAction.RemoveAll(this);
        MuteChatButtonMetadata->OnWidgetGenerated.RemoveAll(this);
    }

    if (UnmuteChatButtonMetadata) 
    {
        UnmuteChatButtonMetadata->ButtonAction.RemoveAll(this);
        UnmuteChatButtonMetadata->OnWidgetGenerated.RemoveAll(this);
    }
}

void UManagingChatSubsystem::UpdateGeneratedWidgets()
{
    // TODO: Reimplement this later. As currently no functionality whether the player is mute or not.
    return;

    // Display mute chat if the player is unmuted.
    if (MuteChatButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(MuteChatButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(true);
            Button->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Display unmute chat if the player is muted.
    if (UnmuteChatButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(UnmuteChatButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(true);
            Button->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UManagingChatSubsystem::MuteChat(FUniqueNetIdPtr TargetUserId)
{
    if (!GetIdentityInterface()) 
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Identity Interface is not valid."));
        return;
    }

    AccelByte::FApiClientPtr ApiClient = GetIdentityInterface()->GetApiClient(0);
    if (!ApiClient)
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Api Client is not valid."));
        return;
    }

    const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(TargetUserId);
    if (!TargetUserABId)
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Target user NetId is not valid."));
        return;
    }

    // TODO: Might need restructure.
    ApiClient->Chat.BlockUser(
        TargetUserABId->GetAccelByteId(), 
        AccelByte::Api::Chat::FChatBlockUserResponse::CreateWeakLambda(this, [this](const FAccelByteModelsChatBlockUserResponse& Result)
        {
            UE_LOG_MANAGINGCHAT(Log, TEXT("Success to mute chat for player %s"), *Result.UserId);

            if (GetPromptSubystem())
            {
                GetPromptSubystem()->PushNotification(FText::FromString(FString("Player text chat is muted")));
            }

            UpdateGeneratedWidgets();
        }),
        FErrorHandler::CreateWeakLambda(this, [this](int32 ErrorCode, const FString& ErrorMessage)
        {
            UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Error %d: %s"), ErrorCode, *ErrorMessage);

            if (GetPromptSubystem())
            {
                GetPromptSubystem()->PushNotification(FText::FromString(FString("Failed to mute player from text chat")));
            }
        })
    );
}

void UManagingChatSubsystem::UnmuteChat(FUniqueNetIdPtr TargetUserId)
{
    if (!GetIdentityInterface())
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Identity Interface is not valid."));
        return;
    }

    AccelByte::FApiClientPtr ApiClient = GetIdentityInterface()->GetApiClient(0);
    if (!ApiClient)
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Api Client is not valid."));
        return;
    }

    const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(TargetUserId);
    if (!TargetUserABId)
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Target user NetId is not valid."));
        return;
    }

    // TODO: Might need restructure.
    ApiClient->Chat.UnblockUser(
        TargetUserABId->GetAccelByteId(),
        AccelByte::Api::Chat::FChatUnblockUserResponse::CreateWeakLambda(this, [this](const FAccelByteModelsChatUnblockUserResponse& Result)
        {
            UE_LOG_MANAGINGCHAT(Log, TEXT("Success to unmute chat for player %s"), *Result.UserId);

            if (GetPromptSubystem())
            {
                GetPromptSubystem()->PushNotification(FText::FromString(FString("Player text chat is unmuted")));
            }

            UpdateGeneratedWidgets();
        }),
        FErrorHandler::CreateWeakLambda(this, [this](int32 ErrorCode, const FString& ErrorMessage)
        {
            UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Error %d: %s"), ErrorCode, *ErrorMessage);

            if (GetPromptSubystem())
            {
                GetPromptSubystem()->PushNotification(FText::FromString(FString("Failed to unmute player from text chat")));
            }
        })
    );
}

FUniqueNetIdPtr UManagingChatSubsystem::GetCurrentDisplayedFriendId()
{
    UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (!ParentWidget)
    {
        return nullptr;
    }

    FUniqueNetIdRepl FriendUserId = nullptr;
    if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
    {
        if (FriendDetailsWidget->GetCachedFriendData() &&
            FriendDetailsWidget->GetCachedFriendData()->UserId &&
            FriendDetailsWidget->GetCachedFriendData()->UserId.IsValid())
        {
            FriendUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
        }
    }

    if (FriendUserId == nullptr || !FriendUserId.IsValid())
    {
        return nullptr;
    }

    return FriendUserId.GetUniqueNetId();
}

FOnlineIdentityAccelBytePtr UManagingChatSubsystem::GetIdentityInterface()
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

FOnlineChatAccelBytePtr UManagingChatSubsystem::GetChatInterface()
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_MANAGINGCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}

UPromptSubsystem* UManagingChatSubsystem::GetPromptSubystem()
{
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}