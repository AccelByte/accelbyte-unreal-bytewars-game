// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "CommonButtonBase.h"
#include "Social/ChatEssentials/UI/ChatWidget.h"

void UPrivateChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PrivateChatSubsystem = GameInstance->GetSubsystem<UPrivateChatSubsystem>();
	ensure(PrivateChatSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

// @@@SNIPSTART PrivateChatWidget.cpp-NativeOnActivated
// @@@MULTISNIP Bind {"selectedLines": ["1-2", "10", "13-21"]}
void UPrivateChatWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset chat widget
	W_Chat->ClearChatMessages();
	W_Chat->ClearInput();

	// Setup widget
	W_Chat->OnSubmitDelegates.AddUObject(this, &ThisClass::SendPrivateChatMessage);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	if (PrivateChatSubsystem)
	{
		PrivateChatSubsystem->GetOnSendPrivateChatCompleteDelegates()->AddUObject(this, &ThisClass::OnSendPrivateChatComplete);
		PrivateChatSubsystem->GetOnPrivateChatMessageReceivedDelegates()->AddUObject(this, &ThisClass::OnPrivateChatMessageReceived);
	}

	// Display default state.
	GetLastPrivateChatMessages();
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP Unbind {"selectedLines": ["1-2", "6", "9-14", "17"]}
void UPrivateChatWidget::NativeOnDeactivated()
{
	PrivateChatRecipientUserId = nullptr;

	// Cleanup widget
	W_Chat->OnSubmitDelegates.RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);

	// Unbind chat events.
	if (PrivateChatSubsystem)
	{
		PrivateChatSubsystem->GetOnSendPrivateChatCompleteDelegates()->RemoveAll(this);
		PrivateChatSubsystem->GetOnPrivateChatMessageReceivedDelegates()->RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

UWidget* UPrivateChatWidget::NativeGetDesiredFocusTarget() const
{
	return W_Chat;
}

void UPrivateChatWidget::SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId)
{
	PrivateChatRecipientUserId = RecipientUserId;
}

// @@@SNIPSTART PrivateChatWidget.cpp-AppendChatMessage
void UPrivateChatWidget::AppendChatMessage(const FChatMessage& Message) const
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

	const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message.GetUserId());
	if (!SenderABId.Get().IsValid())
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot append a private chat message to display. Sender User Id is invalid."));
		return;
	}

	// Construct chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = PrivateChatSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

	// If the sender doesn't have display name, then use the default display name.
	ChatData->Sender = Message.GetNickname();
	if (ChatData->Sender.IsEmpty() || SenderABId.Get().GetAccelByteId().Equals(ChatData->Sender))
	{
		ChatData->Sender = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message.GetUserId().Get());
	}

	// Display chat message.
	W_Chat->AppendChatMessage(ChatData);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatWidget.cpp-SendPrivateChatMessage
// @@@MULTISNIP AddingUI {"selectedLines": ["1-20", "27"]}
void UPrivateChatWidget::SendPrivateChatMessage(const FText& MessageText)
{
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
		MessageText.ToString());
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatWidget.cpp-OnSendPrivateChatComplete
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
	W_Chat->AppendChatMessage(ChatData);
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatWidget.cpp-OnPrivateChatMessageReceived
void UPrivateChatWidget::OnPrivateChatMessageReceived(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message)
{
	// Show the received chat message.
	AppendChatMessage(Message.Get());
}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatWidget.cpp-GetLastPrivateChatMessages
// @@@MULTISNIP AddingUI {"selectedLines": ["1-20", "61"]}
void UPrivateChatWidget::GetLastPrivateChatMessages() const
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

	// Get chat room id.
	const FString ChatRoomId = PrivateChatSubsystem->GetPrivateChatRoomId(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		PrivateChatRecipientUserId);

	// Abort if room id was not found.
	if (ChatRoomId.IsEmpty())
	{
		W_Chat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	// Get last private chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (PrivateChatSubsystem->GetLastPrivateChatMessages(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatRoomId,
		W_Chat->GetMaxChatHistory(),
		OutMessages))
	{
		// Abort if last messages is empty.
		if (OutMessages.IsEmpty())
		{
			W_Chat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
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
		W_Chat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}
// @@@SNIPEND
