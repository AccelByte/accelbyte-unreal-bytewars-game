// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/PrivateChat/UI/PrivateChatWidget.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

// @@@SNIPSTART PrivateChatSubsystem.cpp-Initialize
// @@@MULTISNIP OnSendChatCompleteDelegates {"selectedLines": ["1-2", "66-68", "87-88"]}
// @@@MULTISNIP OnChatPrivateMessageReceivedDelegates {"selectedLines": ["1-2", "66-67", "69-72", "87-88"]}
void UPrivateChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Assign action button to open private chat.
    FTutorialModuleGeneratedWidget* PrivateChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_private_chat"));
    if (PrivateChatButtonMetadata)
    {
        if (!PrivateChatButtonMetadata->OwnerTutorialModule)
        {
            return;
        }

        // Get chat widget class.
        auto WidgetClass = PrivateChatButtonMetadata->OwnerTutorialModule->GetTutorialModuleUIClass();
        if (!WidgetClass)
        {
            return;
        }

        // Open private chat widget and inject the friend user ID to it.
        PrivateChatButtonMetadata->ButtonAction.Clear();
        PrivateChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this, WidgetClass]()
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

            UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
            if (!ParentWidget)
            {
                return;
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
                return;
            }

            if (UPrivateChatWidget* PrivateChatWidget = Cast<UPrivateChatWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass.Get())))
            {
                PrivateChatWidget->SetPrivateChatRecipient(FriendUserId.GetUniqueNetId());
            }
        });
    }

    if (GetChatInterface())
    {
        GetChatInterface()->OnSendChatCompleteDelegates.AddUObject(this, &ThisClass::OnSendPrivateChatComplete);
        GetChatInterface()->OnChatPrivateMessageReceivedDelegates.AddUObject(this, &ThisClass::OnPrivateChatMessageReceived);

        // Push a notification when received chat messages.
        GetChatInterface()->OnChatPrivateMessageReceivedDelegates.AddUObject(this, &ThisClass::PushPrivateChatMessageReceivedNotification);
    
        // Try reconnect chat on disconnected from chat.
        GetChatInterface()->OnConnectChatCompleteDelegates->AddWeakLambda(this, [this](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorMessage)
        {
            if (bWasSuccessful)
            {
                UE_LOG_PRIVATECHAT(Log, TEXT("Success to connect to chat service."));
                ReconnectChatNumTries = 0;
                return;
            }

            ReconnectChat(ErrorMessage);
        });
        GetChatInterface()->OnChatDisconnectedDelegates.AddUObject(this, &ThisClass::ReconnectChat);
    }
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-Deinitialize
// @@@MULTISNIP OnSendChatCompleteDelegates {"selectedLines": ["1-2", "5-7", "12-13"]}
// @@@MULTISNIP OnChatPrivateMessageReceivedDelegates {"selectedLines": ["1-2", "5-6", "8-9", "12-13"]}
void UPrivateChatSubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (GetChatInterface())
    {
        GetChatInterface()->OnSendChatCompleteDelegates.RemoveAll(this);
        GetChatInterface()->OnChatPrivateMessageReceivedDelegates.RemoveAll(this);

        GetChatInterface()->OnConnectChatCompleteDelegates->RemoveAll(this);
        GetChatInterface()->OnChatDisconnectedDelegates.RemoveAll(this);
    }
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-GetPrivateChatRoomId
FString UPrivateChatSubsystem::GetPrivateChatRoomId(const FUniqueNetIdPtr SenderUserId, const FUniqueNetIdPtr RecipientUserId)
{
    if (!GetChatInterface())
    {
        return FString();
    }

    if (!SenderUserId || !RecipientUserId)
    {
        return FString();
    }

    const FUniqueNetIdAccelByteUserPtr SenderABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(SenderUserId);
    const FUniqueNetIdAccelByteUserPtr RecipientABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(RecipientUserId);
    if (!SenderABId || !RecipientABId)
    {
        return FString();
    }

    return GetChatInterface()->PersonalChatTopicId(SenderABId->GetAccelByteId(), RecipientABId->GetAccelByteId());
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-IsMessageFromLocalUser
bool UPrivateChatSubsystem::IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message)
{
    if (!GetChatInterface())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot check whether chat message is from local user or not. Chat Interface is not valid."));
        return false;
    }

    if (!UserId)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot check whether chat message is from local user or not. User NetId is not valid."));
        return false;
    }

    return GetChatInterface()->IsMessageFromLocalUser(UserId.ToSharedRef().Get(), Message, true);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-SendPrivateChatMessage
void UPrivateChatSubsystem::SendPrivateChatMessage(const FUniqueNetIdPtr UserId, const FUniqueNetIdPtr RecipientUserId, const FString& Message)
{
    if (!GetChatInterface())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. Chat Interface is not valid."));
        OnSendPrivateChatComplete(FString(), Message, FString(), false);
        return;
    }

    if (!UserId)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. User NetId is not valid."));
        OnSendPrivateChatComplete(FString(), Message, FString(), false);
        return;
    }

    if (!RecipientUserId)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. Recipient NetId is not valid."));
        OnSendPrivateChatComplete(FString(), Message, FString(), false);
        return;
    }

    GetChatInterface()->SendPrivateChat(UserId.ToSharedRef().Get(), RecipientUserId.ToSharedRef().Get(), Message);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-GetLastPrivateChatMessages
bool UPrivateChatSubsystem::GetLastPrivateChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages)
{
    if (!GetChatInterface())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last chat messages. Chat Interface is not valid."));
        return false;
    }

    if (!UserId)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last chat messages. User NetId is not valid."));
        return false;
    }

    // Abort if not a private chat room.
    if (RoomId.IsEmpty() || GetChatInterface()->GetChatRoomType(RoomId) != EAccelByteChatRoomType::PERSONAL)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last chat messages. Room Id is empty or not a private chat room."));
        return false;
    }

    GetChatInterface()->GetLastMessages(UserId.ToSharedRef().Get(), RoomId, NumMessages, OutMessages);
    UE_LOG_PRIVATECHAT(Log, TEXT("Success to get last chat messages. Number of returned messages: %d"), OutMessages.Num());

    return true;
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-OnSendPrivateChatComplete
void UPrivateChatSubsystem::OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
    // Abort if the room id is not a private chat room.
    if (!GetChatInterface() || GetChatInterface()->GetChatRoomType(RoomId) != EAccelByteChatRoomType::PERSONAL)
    {
        return;
    }

    if (bWasSuccessful)
    {
        UE_LOG_PRIVATECHAT(Log, TEXT("Success to send chat message on Room %s"), *RoomId);
    }
    else
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Failed to send chat message on Room %s"), *RoomId);
    }

    OnSendPrivateChatCompleteDelegates.Broadcast(UserId, MsgBody, RoomId, bWasSuccessful);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-OnPrivateChatMessageReceived
void UPrivateChatSubsystem::OnPrivateChatMessageReceived(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message)
{
    UE_LOG_PRIVATECHAT(Log,
        TEXT("Received private chat message from %s: %s"),
        !Message.Get().GetNickname().IsEmpty() ? *Message.Get().GetNickname() : *UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message->GetUserId().Get()),
        *Message.Get().GetBody());

    OnPrivateChatMessageReceivedDelegates.Broadcast(UserId, Message);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.cpp-PushPrivateChatMessageReceivedNotification
void UPrivateChatSubsystem::PushPrivateChatMessageReceivedNotification(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message)
{
    if (!GetPromptSubsystem())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot push private chat message received notification. Prompt Subsystem is not valid."));
        return;
    }

    const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message->GetUserId());
    if (!SenderABId.Get().IsValid())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot push private chat message received notification. Sender User Id is not valid."));
        return;
    }

    // Only push a notification only if the player is not in the chat menu of the same recipient.
    const UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (const UPrivateChatWidget* PrivateChatWidget = Cast<UPrivateChatWidget>(ActiveWidget))
    {
        const FUniqueNetIdAccelByteUserPtr CurrentRecipientABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PrivateChatWidget->GetPrivateChatRecipient());
        
        if (!CurrentRecipientABId || CurrentRecipientABId->GetAccelByteId().Equals(SenderABId->GetAccelByteId()))
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

    GetPromptSubsystem()->PushNotification(FText::Format(PRIVATE_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
}
// @@@SNIPEND

void UPrivateChatSubsystem::ReconnectChat(FString Message)
{
    if (!GetWorld() || GetWorld()->bIsTearingDown)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Failed to reconnect to chat service. The level world is tearing down."));
        return;
    }

    if (!GetChatInterface())
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Failed to reconnect to chat service. The chat interface is tearing down."));
        return;
    }

    if (!GetIdentityInterface() || GetIdentityInterface()->GetLoginStatus(0) == ELoginStatus::Type::LoggedIn)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Failed to reconnect to chat service. The player is not logged in yet."));
        return;
    }

    if (ReconnectChatNumTries >= ReconnectChatMaxTries)
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("Failed to reconnect to chat service. Number tries to reconnect chat reached %d times limit."), ReconnectChatMaxTries);
        return;
    }

    UE_LOG_PRIVATECHAT(Log, TEXT("Try to reconnect chat due to: %s"), *Message);

    GetChatInterface()->Connect(0);
    ReconnectChatNumTries++;
}

// @@@SNIPSTART PrivateChatSubsystem.cpp-GetChatInterface
FOnlineChatAccelBytePtr UPrivateChatSubsystem::GetChatInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}
// @@@SNIPEND

FOnlineIdentityAccelBytePtr UPrivateChatSubsystem::GetIdentityInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PRIVATECHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

// @@@SNIPSTART PrivateChatSubsystem.cpp-GetPromptSubsystem
UPromptSubsystem* UPrivateChatSubsystem::GetPromptSubsystem() const
{
    const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}
// @@@SNIPEND
