// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void UPrivateChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PrivateChatSubsystem = GameInstance->GetSubsystem<UPrivateChatSubsystem>();
	ensure(PrivateChatSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UPrivateChatWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset private chat messages.
	Lv_ChatMessage->ClearListItems();

	// Reset chat box.
	Edt_ChatMessage->SetText(FText::GetEmpty());

	// Bind event to send private chat message.
	Edt_ChatMessage->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnSendPrivateChatMessageCommited);
	Edt_ChatMessage->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnChatMessageChanged);
	Btn_Send->OnClicked().AddUObject(this, &ThisClass::SendPrivateChatMessage);

	// Bind chat events.
	if (PrivateChatSubsystem)
	{
		PrivateChatSubsystem->GetOnSendPrivateChatCompleteDelegates()->AddUObject(this, &ThisClass::OnSendPrivateChatComplete);
		PrivateChatSubsystem->GetOnPrivateChatMessageReceivedDelegates()->AddUObject(this, &ThisClass::OnPrivateChatMessageReceived);
	}

	// Display default state.
	GetLastPrivateChatMessages();
}

void UPrivateChatWidget::NativeOnDeactivated()
{
	PrivateChatRecipientUserId = nullptr;

	Edt_ChatMessage->OnTextCommitted.Clear();
	Edt_ChatMessage->OnTextChanged.Clear();
	Btn_Send->OnClicked().Clear();

	// Unbind chat events.
	if (PrivateChatSubsystem)
	{
		PrivateChatSubsystem->GetOnSendPrivateChatCompleteDelegates()->RemoveAll(this);
		PrivateChatSubsystem->GetOnPrivateChatMessageReceivedDelegates()->RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}

void UPrivateChatWidget::AppendChatMessage(UChatData* ChatData)
{
	// Display private chat message.
	if (ChatData)
	{
		Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

		Lv_ChatMessage->AddItem(ChatData);
		Lv_ChatMessage->ScrollToBottom();
	}
}

void UPrivateChatWidget::AppendChatMessage(const FChatMessage& Message)
{
	if (!PrivateChatSubsystem)
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot append a private chat message to display. Private Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot append a private chat message to display. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot append a private chat message to display. LocalPlayer is not valid."));
		return;
	}

	// Display chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Sender = Message.GetNickname();
	if (ChatData->Sender.IsEmpty())
	{
		ChatData->Sender = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message.GetUserId().Get());	
	}
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = PrivateChatSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

	AppendChatMessage(ChatData);
}

void UPrivateChatWidget::SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId)
{
	PrivateChatRecipientUserId = RecipientUserId;
}

void UPrivateChatWidget::SendPrivateChatMessage()
{
	// Refresh text pointer.
	Edt_ChatMessage->SetText(Edt_ChatMessage->GetText());

	// Don't send empty message.
	if (Edt_ChatMessage->GetText().IsEmpty())
	{
		return;
	}

	if (!PrivateChatSubsystem)
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. Private Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot send private chat message. LocalPlayer is not valid."));
		return;
	}

	// Send private private chat message.
	PrivateChatSubsystem->SendPrivateChatMessage(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		PrivateChatRecipientUserId,
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

void UPrivateChatWidget::OnSendPrivateChatMessageCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Check if commit method is valid.
	if (CommitMethod != ETextCommit::Type::OnEnter || Text.IsEmpty())
	{
		return;
	}

	SendPrivateChatMessage();
}

void UPrivateChatWidget::OnChatMessageChanged(const FText& Text)
{
	// Disable the send button if it is empty.
	Btn_Send->SetIsEnabled(!Text.IsEmpty());

	// Limit the chat message length.
	Edt_ChatMessage->SetText(FText::FromString(Edt_ChatMessage->GetText().ToString().Left(MaxMessageLength)));
}

void UPrivateChatWidget::GetLastPrivateChatMessages()
{
	if (!PrivateChatSubsystem)
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last private chat messages. Private Chat subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last private chat messages. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot get last private chat messages. LocalPlayer is not valid."));
		return;
	}

	// Show loading message.
	Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Get chat room id.
	const FString ChatRoomId = PrivateChatSubsystem->GetPrivateChatRoomId(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		PrivateChatRecipientUserId);

	// Abort if room id was not found.
	if (ChatRoomId.IsEmpty())
	{
		Ws_ChatMessage->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	// Get last private chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (PrivateChatSubsystem->GetLastPrivateChatMessages(
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

void UPrivateChatWidget::OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
	// Abort and push a notification if failed.
	if (!bWasSuccessful)
	{
		if (PromptSubsystem)
		{
			PromptSubsystem->PushNotification(SEND_CHAT_FAILED_MESSAGE);
		}

		return;
	}

	// Display the chat if success.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Message = MsgBody;
	ChatData->bIsSenderLocal = true;
	AppendChatMessage(ChatData);
}

void UPrivateChatWidget::OnPrivateChatMessageReceived(const FUniqueNetId& Sender, const TSharedRef<FChatMessage>& Message)
{
	// Show the received chat message.
	AppendChatMessage(Message.Get());
}
