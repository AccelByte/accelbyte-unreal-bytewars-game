// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Social/SessionChat/UI/SessionChatWidget.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void USessionChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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

            if (USessionChatWidget* SessionChatWidget = Cast<USessionChatWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass.Get())))
            {
                SessionChatWidget->SetDefaultChatType(EAccelByteChatRoomType::PARTY_V2);
            }
        });
    }
    
    if (GetChatInterface()) 
    {
        GetChatInterface()->OnSendChatCompleteDelegates.AddUObject(this, &ThisClass::OnSendChatComplete);
        GetChatInterface()->OnChatRoomMessageReceivedDelegates.AddUObject(this, &ThisClass::OnChatRoomMessageReceived);

        // Push a notification when received chat messages.
        GetChatInterface()->OnChatRoomMessageReceivedDelegates.AddUObject(this, &ThisClass::PushChatRoomMessageReceivedNotification);
    }
}

void USessionChatSubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (GetChatInterface())
    {
        GetChatInterface()->OnSendChatCompleteDelegates.RemoveAll(this);
        GetChatInterface()->OnChatRoomMessageReceivedDelegates.RemoveAll(this);
    }
}

FString USessionChatSubsystem::GetChatRoomIdBasedOnType(const EAccelByteChatRoomType ChatRoomType)
{
    FString ChatRoomId;

    switch (ChatRoomType)
    {
    case EAccelByteChatRoomType::SESSION_V2:
        ChatRoomId = GetGameSessionChatRoomId();
        break;
    case EAccelByteChatRoomType::PARTY_V2:
        ChatRoomId = GetPartyChatRoomId();
        break;
    }

    return ChatRoomId;
}

FString USessionChatSubsystem::GetGameSessionChatRoomId()
{
    if (!GetChatInterface())
    {
        return FString();
    }

    if (!GetSessionInterface()) 
    {
        return FString();
    }

    const FNamedOnlineSession* GameSession = GetSessionInterface()->GetNamedSession(NAME_GameSession);
    if (!GameSession)
    {
        return FString();
    }

    return GetChatInterface()->SessionV2IdToChatTopicId(GameSession->GetSessionIdStr());
}

FString USessionChatSubsystem::GetPartyChatRoomId()
{
    if (!GetChatInterface())
    {
        return FString();
    }

    if (!GetSessionInterface())
    {
        return FString();
    }

    const FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    if (!PartySession)
    {
        return FString();
    }

    return GetChatInterface()->PartyV2IdToChatTopicId(PartySession->GetSessionIdStr());
}

void USessionChatSubsystem::SendChatMessage(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const FString& Message)
{
    if (!GetChatInterface()) 
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. Chat Interface is not valid."));
        OnSendChatComplete(FString(), Message, RoomId, false);
        return;
    }

    if (!UserId) 
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. User NetId is not valid."));
        OnSendChatComplete(FString(), Message, RoomId, false);
        return;
    }

    if (RoomId.IsEmpty()) 
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. Room Id is empty."));
        OnSendChatComplete(FString(), Message, RoomId, false);
        return;
    }

    GetChatInterface()->SendRoomChat(UserId.ToSharedRef().Get(), RoomId, Message);
}

bool USessionChatSubsystem::GetLastChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages)
{
    if (!GetChatInterface())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. Chat Interface is not valid."));
        return false;
    }

    if (!UserId)
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. User NetId is not valid."));
        return false;
    }

    if (RoomId.IsEmpty())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. Room Id is empty."));
        return false;
    }

    GetChatInterface()->GetLastMessages(UserId.ToSharedRef().Get(), RoomId, NumMessages, OutMessages);
    UE_LOG_SESSIONCHAT(Log, TEXT("Success to get last chat messages. Returned messages: %d"), OutMessages.Num());

    return true;
}

bool USessionChatSubsystem::IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message)
{
    if (!GetChatInterface())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot check whether chat message is from local user or not. Chat Interface is not valid."));
        return false;
    }

    if (!UserId)
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot check whether chat message is from local user or not. User NetId is not valid."));
        return false;
    }

    return GetChatInterface()->IsMessageFromLocalUser(UserId.ToSharedRef().Get(), Message, true);
}

EAccelByteChatRoomType USessionChatSubsystem::GetChatRoomType(const FString& RoomId)
{
    if (!GetChatInterface())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get chat room type for Room Id: %s"), *RoomId);

        return EAccelByteChatRoomType::NORMAL;
    }

    return GetChatInterface()->GetChatRoomType(RoomId);
}

void USessionChatSubsystem::OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
    if (bWasSuccessful) 
    {
        UE_LOG_SESSIONCHAT(Log, TEXT("Success to send chat message on Room %s"), *RoomId);
    }
    else 
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Failed to send chat message on Room %s"), *RoomId);
    }

    OnSendChatCompleteDelegates.Broadcast(UserId, MsgBody, RoomId, bWasSuccessful);
}

void USessionChatSubsystem::OnChatRoomMessageReceived(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
    UE_LOG_SESSIONCHAT(Log, 
        TEXT("Received chat message from %s on Room %s: %s"),
        !Message.Get().GetNickname().IsEmpty() ? *Message.Get().GetNickname() : *UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message->GetUserId().Get()),
        *RoomId,
        *Message.Get().GetBody());

    OnChatRoomMessageReceivedDelegates.Broadcast(UserId, RoomId, Message);
}

void USessionChatSubsystem::PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
    if (!GetChatInterface())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Chat Interface is not valid."));
        return;
    }

    if (!GetPromptSubsystem())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Prompt Subsystem is not valid."));
        return;
    }

    const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message->GetUserId());
    if (!SenderABId.Get().IsValid())
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Sender User Id is not valid."));
        return;
    }

    // Only push a notification only if the player is not in the chat menu of the same type.
    const EAccelByteChatRoomType ChatRoomType = GetChatInterface()->GetChatRoomType(RoomId);
    const UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (const USessionChatWidget* SessionWidget = Cast<USessionChatWidget>(ActiveWidget))
    {
        if (SessionWidget->GetCurrentChatType() == ChatRoomType)
        {
            return;
        }
    }

    // If the sender doesn't have display name, then use the default display name.
    FString SenderStr = Message.Get().GetNickname();
    if (SenderStr.IsEmpty() || SenderABId.Get().GetAccelByteId().Equals(SenderStr))
    {
        SenderStr = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message->GetUserId().Get());
    }

    switch (ChatRoomType)
    {
    case EAccelByteChatRoomType::SESSION_V2:
        // Push the notification only if the player already travelled to online session.
        if (GetWorld() && GetWorld()->GetNetMode() != ENetMode::NM_Standalone)
        {
            GetPromptSubsystem()->PushNotification(FText::Format(GAMESESSION_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
        }
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

FOnlineChatAccelBytePtr USessionChatSubsystem::GetChatInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}

FOnlineSessionV2AccelBytePtr USessionChatSubsystem::GetSessionInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
}

UPromptSubsystem* USessionChatSubsystem::GetPromptSubsystem() const
{
    const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}
