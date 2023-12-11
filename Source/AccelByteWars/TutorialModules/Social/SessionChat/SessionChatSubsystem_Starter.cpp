// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Social/SessionChat/UI/SessionChatWidget_Starter.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void USessionChatSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Assign action button to open party chat.
    if (FTutorialModuleGeneratedWidget* PartyChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_partychat")))
    {
        if (!PartyChatButtonMetadata->OwnerTutorialModule)
        {
            return;
        }

        auto WidgetClass = PartyChatButtonMetadata->OwnerTutorialModule->GetTutorialModuleUIClass();
        if (!WidgetClass)
        {
            return;
        }

        PartyChatButtonMetadata->ButtonAction.Clear();
        PartyChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this, WidgetClass]()
        {
            UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
            if (!GameInstance)
            {
                return;
            }

            UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
            if (!BaseUIWidget)
            {
                return;
            }

            if (USessionChatWidget_Starter* SessionChatWidget = Cast<USessionChatWidget_Starter>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass.Get())))
            {
                SessionChatWidget->SetDefaultChatType(EAccelByteChatRoomType::PARTY_V2);
            }
        });
    }

    // TODO: Bind session chat events here.
}

void USessionChatSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    // TODO: Unbind session chat events here.
}

void USessionChatSubsystem_Starter::PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
    if (!GetChatInterface() || !GetPromptSubsystem())
    {
        return;
    }

    // Only push a notification only if the player is not in the chat menu.
    const UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (Cast<USessionChatWidget_Starter>(ActiveWidget))
    {
        return;
    }

    FString SenderStr = Message.Get().GetNickname();
    if (SenderStr.IsEmpty())
    {
        SenderStr = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Sender);
    }

    switch (GetChatInterface()->GetChatRoomType(RoomId))
    {
    case EAccelByteChatRoomType::SESSION_V2:
        GetPromptSubsystem()->PushNotification(FText::Format(GAMESESSION_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
        break;
    case EAccelByteChatRoomType::PARTY_V2:
        GetPromptSubsystem()->PushNotification(FText::Format(PARTY_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
        break;
    default:
        break;
    }

    // The push notification is not shown to the gameplay level. Instead, the game will show exclamation mark on the chat button.
    if (const FTutorialModuleGeneratedWidget* SessionChatGameplayButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_session_chat_gameplay")))
    {
        if (UAccelByteWarsButtonBase* Button = Cast<UAccelByteWarsButtonBase>(SessionChatGameplayButtonMetadata->GenerateWidgetRef))
        {
            Button->ToggleExclamationMark(true);
        }
    }
}

FOnlineChatAccelBytePtr USessionChatSubsystem_Starter::GetChatInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}

FOnlineSessionV2AccelBytePtr USessionChatSubsystem_Starter::GetSessionInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
}

UPromptSubsystem* USessionChatSubsystem_Starter::GetPromptSubsystem() const
{
    const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}

#pragma region Module Session Chat Function Definitions

// TODO: Add your Module Session Chat function definitions here.

#pragma endregion