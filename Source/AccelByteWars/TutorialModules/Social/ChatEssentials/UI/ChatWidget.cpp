// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChatWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	ChatEssentialsSubsystem = GameInstance->GetSubsystem<UChatEssentialsSubsystem>();
	ensure(ChatEssentialsSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UChatWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset chat messages.
	Lv_SessionChatMessage->ClearListItems();
	Lv_PartyChatMessage->ClearListItems();

	// Reset chat box.
	Edt_ChatMessage->SetText(FText::GetEmpty());

	// Bind event to send chat message.
	Edt_ChatMessage->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnSendChatMessageCommited);
	Btn_Send->OnClicked().AddUObject(this, &ThisClass::SendChatMessage);

	// Bind event to switch between chat message type.
	Btn_SessionChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::SESSION_V2);
	Btn_PartyChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::PARTY_V2);

	// Bind chat events.
	if (ChatEssentialsSubsystem) 
	{
		ChatEssentialsSubsystem->GetOnSendChatCompleteDelegates()->AddUObject(this, &ThisClass::OnSendChatComplete);
		ChatEssentialsSubsystem->GetOnChatRoomMessageReceivedDelegates()->AddUObject(this, &ThisClass::OnChatRoomMessageReceived);
	}

	// Display default state.
	SwitchChatMessageType(EAccelByteChatRoomType::SESSION_V2);
}

void UChatWidget::NativeOnDeactivated()
{
	Edt_ChatMessage->OnTextCommitted.Clear();
	Btn_Send->OnClicked().Clear();
	
	Btn_SessionChat->OnClicked().Clear();
	Btn_PartyChat->OnClicked().Clear();

	// Unbind chat events.
	if (ChatEssentialsSubsystem)
	{
		ChatEssentialsSubsystem->GetOnSendChatCompleteDelegates()->RemoveAll(this);
		ChatEssentialsSubsystem->GetOnChatRoomMessageReceivedDelegates()->RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}

void UChatWidget::SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType)
{
	// Switch chat message active panel based on type.
	switch(ChatRoomType) 
	{
	case EAccelByteChatRoomType::SESSION_V2:
		Ws_ChatMessageType->SetActiveWidget(Vb_SessionChat);
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		Ws_ChatMessageType->SetActiveWidget(Vb_PartyChat);
		break;
	}

	CurrentChatRoomType = ChatRoomType;

	// Try to display last chat messages if the current one is empty.
	const UListView* TargetContainer = GetChatMessageContainerBasedOnChatRoomType(ChatRoomType);
	if (TargetContainer && TargetContainer->GetListItems().IsEmpty())
	{
		GetLastChatMessages(ChatRoomType);
	}
}

UListView* UChatWidget::GetChatMessageContainerBasedOnChatRoomType(const EAccelByteChatRoomType ChatRoomType)
{
	UListView* TargetContainer = nullptr;

	switch (ChatRoomType)
	{
	case EAccelByteChatRoomType::SESSION_V2:
		TargetContainer = Lv_SessionChatMessage;
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		TargetContainer = Lv_PartyChatMessage;
		break;
	}

	return TargetContainer;
}

void UChatWidget::AppendChatMessage(UChatData* ChatData, const EAccelByteChatRoomType ChatRoomType)
{
	// Display chat message.
	UListView* TargetContainer = GetChatMessageContainerBasedOnChatRoomType(ChatRoomType);
	if (TargetContainer && ChatData)
	{
		TargetContainer->AddItem(ChatData);
		TargetContainer->ScrollToBottom();
	}
}

void UChatWidget::AppendChatMessage(const FChatMessage& Message, const EAccelByteChatRoomType ChatRoomType)
{
	if (!ChatEssentialsSubsystem)
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot append a chat message to display. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot append a chat message to display. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot append a chat message to display. LocalPlayer is not valid."));
		return;
	}

	// Display chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Sender = Message.GetNickname();
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = ChatEssentialsSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

	AppendChatMessage(ChatData, ChatRoomType);
}

void UChatWidget::SendChatMessage()
{
	// Refresh text pointer.
	Edt_ChatMessage->SetText(Edt_ChatMessage->GetText());

	// Don't send empty message.
	if (Edt_ChatMessage->GetText().IsEmpty())
	{
		return;
	}

	if (!ChatEssentialsSubsystem)
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot send chat message. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot send chat message. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot send chat message. LocalPlayer is not valid."));
		return;
	}

	// Send room chat message.
	ChatEssentialsSubsystem->SendChatMessage(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatEssentialsSubsystem->GetChatRoomIdBasedOnType(CurrentChatRoomType),
		Edt_ChatMessage->GetText().ToString());

	// Clear text after sending chat.
	Edt_ChatMessage->SetText(FText::GetEmpty());
	Edt_ChatMessage->SetUserFocus(GetOwningPlayer());
}

void UChatWidget::OnSendChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Check if commit method is valid.
	if (CommitMethod != ETextCommit::Type::OnEnter || Text.IsEmpty())
	{
		return;
	}

	SendChatMessage();
}

void UChatWidget::GetLastChatMessages(const EAccelByteChatRoomType ChatRoomType)
{
	if (!ChatEssentialsSubsystem) 
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot get last chat messages. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot get last chat messages. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot get last chat messages. LocalPlayer is not valid."));
		return;
	}

	FString ChatRoomId = ChatEssentialsSubsystem->GetChatRoomIdBasedOnType(ChatRoomType);

	// Show warning.
	// TODO: Might need restructure.
	if (ChatRoomId.IsEmpty()) 
	{
		if (PromptSubsystem)
		{
			FString WarningMessage;
			switch(ChatRoomType) 
			{
			case EAccelByteChatRoomType::SESSION_V2:
				WarningMessage = FString("You have not joined any game session chat.");
				break;
			case EAccelByteChatRoomType::PARTY_V2:
				WarningMessage = FString("You have not joined any party session chat.");
				break;
			}

			PromptSubsystem->PushNotification(FText::FromString(WarningMessage));
		}

		return;
	}

	// Get last chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (ChatEssentialsSubsystem->GetLastChatMessages(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatRoomId,
		100,
		OutMessages)) 
	{
		// Display last chat messages.
		Algo::Reverse(OutMessages);
		for(const TSharedRef<FChatMessage>& Message : OutMessages) 
		{
			AppendChatMessage(Message.Get(), ChatRoomType);
		}
	}
}

void UChatWidget::OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
	if (bWasSuccessful && ChatEssentialsSubsystem)
	{
		UChatData* ChatData = NewObject<UChatData>();
		ChatData->Message = MsgBody;
		ChatData->bIsSenderLocal = true;

		AppendChatMessage(ChatData, ChatEssentialsSubsystem->GetChatRoomType(RoomId));
	}
	else
	{
		// TODO: Might need restructure.
		if (PromptSubsystem) 
		{
			PromptSubsystem->PushNotification(
				FText::FromString(FString("Failed to send chat message")), 
				FString());
		}
	}
}

void UChatWidget::OnChatRoomMessageReceived(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
	if (!ChatEssentialsSubsystem)
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot display received chat message. Chat Essentials subsystem is not valid."));
		return;
	}

	// Replace sender name with default player name if it is empty.
	if (Message.Get().GetNickname().IsEmpty()) 
	{
		UChatData* ChatData = NewObject<UChatData>();
		ChatData->Sender = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Sender);
		ChatData->Message = Message.Get().GetBody();
		ChatData->bIsSenderLocal = ChatEssentialsSubsystem->IsMessageFromLocalUser(Sender.AsShared(), Message.Get());;

		AppendChatMessage(ChatData, ChatEssentialsSubsystem->GetChatRoomType(RoomId));
	}
	// If not, show the chat as is.
	else 
	{
		AppendChatMessage(Message.Get(), ChatEssentialsSubsystem->GetChatRoomType(RoomId));
	}
}
