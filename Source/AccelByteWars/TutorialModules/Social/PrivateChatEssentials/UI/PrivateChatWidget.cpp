// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void UPrivateChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PrivateChatEssentialsSubsystem = GameInstance->GetSubsystem<UPrivateChatEssentialsSubsystem>();
	ensure(PrivateChatEssentialsSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UPrivateChatWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset private chat messages.
	Lv_PrivateChatMessage->ClearListItems();

	// Reset chat box.
	Edt_ChatMessage->SetText(FText::GetEmpty());

	// Bind event to send private chat message.
	Edt_ChatMessage->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnSendPrivateChatMessageCommited);
	Btn_Send->OnClicked().AddUObject(this, &ThisClass::SendPrivateChatMessage);

	// Bind chat events.
	if (PrivateChatEssentialsSubsystem)
	{
		PrivateChatEssentialsSubsystem->GetOnSendPrivateChatCompleteDelegates()->AddUObject(this, &ThisClass::OnSendPrivateChatComplete);
		PrivateChatEssentialsSubsystem->GetOnPrivateChatMessageReceivedDelegates()->AddUObject(this, &ThisClass::OnPrivateChatMessageReceived);
	}

	// Display default state.
	GetLastPrivateChatMessages();
}

void UPrivateChatWidget::NativeOnDeactivated()
{
	PrivateChatRecipientUserId = nullptr;

	Edt_ChatMessage->OnTextCommitted.Clear();
	Btn_Send->OnClicked().Clear();

	// Unbind chat events.
	if (PrivateChatEssentialsSubsystem)
	{
		PrivateChatEssentialsSubsystem->GetOnSendPrivateChatCompleteDelegates()->RemoveAll(this);
		PrivateChatEssentialsSubsystem->GetOnPrivateChatMessageReceivedDelegates()->RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}

void UPrivateChatWidget::AppendChatMessage(UChatData* ChatData)
{
	// Display private chat message.
	if (ChatData)
	{
		Lv_PrivateChatMessage->AddItem(ChatData);
		Lv_PrivateChatMessage->ScrollToBottom();
	}
}

void UPrivateChatWidget::AppendChatMessage(const FChatMessage& Message)
{
	if (!PrivateChatEssentialsSubsystem)
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot append a private chat message to display. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot append a private chat message to display. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot append a private chat message to display. LocalPlayer is not valid."));
		return;
	}

	// Display chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Sender = Message.GetNickname();
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = PrivateChatEssentialsSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

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

	if (!PrivateChatEssentialsSubsystem)
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot send private chat message. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot send private chat message. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot send private chat message. LocalPlayer is not valid."));
		return;
	}

	// Send private private chat message.
	PrivateChatEssentialsSubsystem->SendPrivateChatMessage(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		PrivateChatRecipientUserId,
		Edt_ChatMessage->GetText().ToString());

	// Clear text after sending chat.
	Edt_ChatMessage->SetText(FText::GetEmpty());
	Edt_ChatMessage->SetUserFocus(GetOwningPlayer());
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

void UPrivateChatWidget::GetLastPrivateChatMessages()
{
	if (!PrivateChatEssentialsSubsystem)
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot get last private chat messages. Chat Essentials subsystem is not valid."));
		return;
	}

	if (!ensure(GetOwningPlayer()))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot get last private chat messages. PlayerController is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_PRIVATECHATESSENTIALS(Warning, TEXT("Cannot get last private chat messages. LocalPlayer is not valid."));
		return;
	}

	const FString ChatRoomId = PrivateChatEssentialsSubsystem->GetPrivateChatRoomId(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		PrivateChatRecipientUserId);

	// Show warning.
	// TODO: Might need restructure.
	if (ChatRoomId.IsEmpty())
	{
		if (PromptSubsystem)
		{
			FString WarningMessage = FString("You have not joined any private chat. Try to chat through friends menu.");
			PromptSubsystem->PushNotification(FText::FromString(WarningMessage));
		}

		return;
	}

	// Get last private chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (PrivateChatEssentialsSubsystem->GetLastPrivateChatMessages(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatRoomId,
		100,
		OutMessages))
	{
		// Display last private chat messages.
		Algo::Reverse(OutMessages);
		for (const TSharedRef<FChatMessage>& Message : OutMessages)
		{
			AppendChatMessage(Message.Get());
		}
	}
}

void UPrivateChatWidget::OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
{
	if (bWasSuccessful && PrivateChatEssentialsSubsystem)
	{
		UChatData* ChatData = NewObject<UChatData>();
		ChatData->Message = MsgBody;
		ChatData->bIsSenderLocal = true;

		AppendChatMessage(ChatData);
	}
	else
	{
		// TODO: Might need restructure.
		if (PromptSubsystem)
		{
			PromptSubsystem->PushNotification(
				FText::FromString(FString("Failed to send private chat message")),
				FString());
		}
	}
}

void UPrivateChatWidget::OnPrivateChatMessageReceived(const FUniqueNetId& Sender, const TSharedRef<FChatMessage>& Message)
{
	AppendChatMessage(Message.Get());
}
