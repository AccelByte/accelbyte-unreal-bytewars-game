// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void USessionChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	SessionChatSubsystem = GameInstance->GetSubsystem<USessionChatSubsystem>();
	ensure(SessionChatSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void USessionChatWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset cache.
	Lv_ChatMessage = nullptr;
	Ws_ChatMessage = nullptr;

	// Reset chat messages.
	Lv_GameSessionChatMessage->ClearListItems();
	Lv_PartyChatMessage->ClearListItems();

	// Reset chat box.
	Edt_ChatMessage->SetText(FText::GetEmpty());

	// Bind event to send chat message.
	Edt_ChatMessage->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnSendChatMessageCommited);
	Edt_ChatMessage->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnChatMessageChanged);
	Btn_Send->OnClicked().AddUObject(this, &ThisClass::SendChatMessage);

	// Bind event to switch between chat message type.
	Btn_SessionChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::SESSION_V2);
	Btn_PartyChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::PARTY_V2);

	// Bind chat events.
	if (SessionChatSubsystem) 
	{
		SessionChatSubsystem->GetOnSendChatCompleteDelegates()->AddUObject(this, &ThisClass::OnSendChatComplete);
		SessionChatSubsystem->GetOnChatRoomMessageReceivedDelegates()->AddUObject(this, &ThisClass::OnChatRoomMessageReceived);
	}

	SwitchChatMessageType(CurrentChatRoomType);
}

void USessionChatWidget::NativeOnDeactivated()
{
	CurrentChatRoomType = EAccelByteChatRoomType::SESSION_V2;

	Edt_ChatMessage->OnTextCommitted.Clear();
	Edt_ChatMessage->OnTextChanged.Clear();
	Btn_Send->OnClicked().Clear();
	
	Btn_SessionChat->OnClicked().Clear();
	Btn_PartyChat->OnClicked().Clear();

	// Unbind chat events.
	if (SessionChatSubsystem)
	{
		SessionChatSubsystem->GetOnSendChatCompleteDelegates()->RemoveAll(this);
		SessionChatSubsystem->GetOnChatRoomMessageReceivedDelegates()->RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}

void USessionChatWidget::SetDefaultChatType(const EAccelByteChatRoomType ChatRoomType)
{
	CurrentChatRoomType = ChatRoomType;
}

void USessionChatWidget::SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType)
{
	// Switch chat message active panel based on type.
	switch(ChatRoomType) 
	{
	case EAccelByteChatRoomType::SESSION_V2:
		Ws_ChatMessageType->SetActiveWidget(Vb_GameSessionChat);
		Ws_ChatMessageTypeButton->SetActiveWidget(Btn_PartyChat);

		Lv_ChatMessage = Lv_GameSessionChatMessage;
		Ws_ChatMessage = Ws_GameSessionChatMessage;
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		Ws_ChatMessageType->SetActiveWidget(Vb_PartyChat);
		Ws_ChatMessageTypeButton->SetActiveWidget(Btn_SessionChat);

		Lv_ChatMessage = Lv_PartyChatMessage;
		Ws_ChatMessage = Ws_PartyChatMessage;
		break;
	}

	CurrentChatRoomType = ChatRoomType;

	// Try to display last chat messages if the current one is empty.
	if (Lv_ChatMessage && Lv_ChatMessage->GetListItems().IsEmpty())
	{
		GetLastChatMessages();
	}
}

void USessionChatWidget::AppendChatMessage(UChatData* ChatData)
{
	// Display chat message.
	if (Lv_ChatMessage && ChatData)
	{
		Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

		Lv_ChatMessage->AddItem(ChatData);
		Lv_ChatMessage->ScrollToBottom();
	}
}

void USessionChatWidget::AppendChatMessage(const FChatMessage& Message)
{
	if (!SessionChatSubsystem)
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot append a chat message to display. Session Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot append a chat message to display. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot append a chat message to display. LocalPlayer is not valid."));
		return;
	}

	// Display chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Sender = Message.GetNickname();
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = SessionChatSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

	AppendChatMessage(ChatData);
}

void USessionChatWidget::SendChatMessage()
{
	// Refresh text pointer.
	Edt_ChatMessage->SetText(Edt_ChatMessage->GetText());

	// Don't send empty message.
	if (Edt_ChatMessage->GetText().IsEmpty())
	{
		return;
	}

	if (!SessionChatSubsystem)
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. Session Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot send chat message. LocalPlayer is not valid."));
		return;
	}

	// Send room chat message.
	SessionChatSubsystem->SendChatMessage(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		SessionChatSubsystem->GetChatRoomIdBasedOnType(CurrentChatRoomType),
		Edt_ChatMessage->GetText().ToString());

	// Clear text after sending chat.
	Edt_ChatMessage->SetText(FText::GetEmpty());
	Edt_ChatMessage->SetUserFocus(GetOwningPlayer());

	// Cooldown to prevent spamming.
	Edt_ChatMessage->SetIsEnabled(false);
	Btn_Send->SetIsEnabled(false);
	GetGameInstance()->GetTimerManager().SetTimer(
		SendChatDelayTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Edt_ChatMessage->SetIsEnabled(true);
			Btn_Send->SetIsEnabled(true);
		}),
		SendChatCooldown, false, SendChatCooldown);
}

void USessionChatWidget::OnSendChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Check if commit method is valid.
	if (CommitMethod != ETextCommit::Type::OnEnter || Text.IsEmpty())
	{
		return;
	}

	SendChatMessage();
}

void USessionChatWidget::OnChatMessageChanged(const FText& Text)
{
	// Disable the send button if it is empty.
	Btn_Send->SetIsEnabled(!Text.IsEmpty());

	// Limit the chat message length.
	Edt_ChatMessage->SetText(FText::FromString(Edt_ChatMessage->GetText().ToString().Left(MaxMessageLength)));
}

void USessionChatWidget::GetLastChatMessages()
{
	if (!SessionChatSubsystem) 
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. Session Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. LocalPlayer is not valid."));
		return;
	}

	// Show loading message.
	if (!ensure(Ws_ChatMessage))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. Chat message container is not valid."));
		return;
	}
	Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Get chat room id.
	FString ChatRoomId = SessionChatSubsystem->GetChatRoomIdBasedOnType(CurrentChatRoomType);

	// Abort if room id was not found.
	if (ChatRoomId.IsEmpty())
	{
		Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	// Get last chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (SessionChatSubsystem->GetLastChatMessages(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatRoomId,
		MaxChatHistory,
		OutMessages)) 
	{
		// Abort if last messages is empty.
		if (OutMessages.IsEmpty())
		{
			Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
			return;
		}

		// Display last chat messages.
		Algo::Reverse(OutMessages);
		for (const TSharedRef<FChatMessage>& Message : OutMessages)
		{
			AppendChatMessage(Message.Get());
		}
	}
	// Show error message if failed.
	else
	{
		Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}

void USessionChatWidget::OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
	if (!SessionChatSubsystem)
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot display a sent chat message. Session Chat subsystem is not valid."));
		return;
	}

	// Only show the chat if the type is valid.
	if (SessionChatSubsystem->GetChatRoomType(RoomId) != CurrentChatRoomType)
	{
		return;
	}

	// Display the chat if success.
	if (bWasSuccessful && SessionChatSubsystem)
	{
		UChatData* ChatData = NewObject<UChatData>();
		ChatData->Message = MsgBody;
		ChatData->bIsSenderLocal = true;

		AppendChatMessage(ChatData);
	}
	// Push a notification if failed.
	else
	{
		if (PromptSubsystem) 
		{
			PromptSubsystem->PushNotification(SEND_CHAT_FAILED_MESSAGE);
		}
	}
}

void USessionChatWidget::OnChatRoomMessageReceived(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
	if (!SessionChatSubsystem)
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot display a received chat message. Session Chat subsystem is not valid."));
		return;
	}

	// Only show the chat if the type is valid.
	if (SessionChatSubsystem->GetChatRoomType(RoomId) != CurrentChatRoomType) 
	{
		return;
	}

	// Replace sender name with default player name if it is empty.
	if (Message.Get().GetNickname().IsEmpty()) 
	{
		UChatData* ChatData = NewObject<UChatData>();
		ChatData->Sender = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Sender);
		ChatData->Message = Message.Get().GetBody();
		ChatData->bIsSenderLocal = SessionChatSubsystem->IsMessageFromLocalUser(Sender.AsShared(), Message.Get());;

		AppendChatMessage(ChatData);
	}
	// If not, show the chat as is.
	else 
	{
		AppendChatMessage(Message.Get());
	}
}
