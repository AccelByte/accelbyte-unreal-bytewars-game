// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Social/ChatEssentials/UI/ChatWidget.h"

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
	
	// Reset chat widget
	W_GameSessionChat->ClearChatMessages();
	W_GameSessionChat->ClearInput();
	W_PartyChat->ClearChatMessages();
	W_PartyChat->ClearInput();

	// Bind event to switch between chat message type.
	Btn_SessionChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::SESSION_V2);
	Btn_PartyChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::PARTY_V2);

	// Setup widget
	W_GameSessionChat->OnSubmitDelegates.AddUObject(this, &ThisClass::SendChatMessage);
	W_PartyChat->OnSubmitDelegates.AddUObject(this, &ThisClass::SendChatMessage);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

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

	// Unbind chat events.
	if (SessionChatSubsystem)
	{
		SessionChatSubsystem->GetOnSendChatCompleteDelegates()->RemoveAll(this);
		SessionChatSubsystem->GetOnChatRoomMessageReceivedDelegates()->RemoveAll(this);
	}

	// Cleanup widget
	Btn_SessionChat->OnClicked().Clear();
	Btn_PartyChat->OnClicked().Clear();
	W_GameSessionChat->OnSubmitDelegates.RemoveAll(this);
	W_PartyChat->OnSubmitDelegates.RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);

	Super::NativeOnDeactivated();
}

UWidget* USessionChatWidget::NativeGetDesiredFocusTarget() const
{
	UWidget* TargetWidget = nullptr;

	switch(CurrentChatRoomType)
	{
	case EAccelByteChatRoomType::SESSION_V2:
		TargetWidget = W_GameSessionChat;
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		TargetWidget = W_PartyChat;
		break;
	}

	return TargetWidget;
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
		Ws_ChatMessageType->SetActiveWidget(W_GameSessionChatOuter);
		W_ActiveChat = W_GameSessionChat;
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		Ws_ChatMessageType->SetActiveWidget(W_PartyChatOuter);
		W_ActiveChat = W_PartyChat;
		break;
	}

	CurrentChatRoomType = ChatRoomType;

	// Try to display last chat messages if the current one is empty.
	if (W_ActiveChat)
	{
		W_ActiveChat->ClearChatMessages();
		W_ActiveChat->ClearInput();
		GetLastChatMessages();
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

	const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message.GetUserId());
	if (!SenderABId.Get().IsValid())
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot append a chat message to display. Sender User Id is invalid."));
		return;
	}

	// Display chat message.
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Message = Message.GetBody();
	ChatData->bIsSenderLocal = SessionChatSubsystem->IsMessageFromLocalUser(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), Message);

	// If the sender doesn't have display name, then use the default display name.
	ChatData->Sender = Message.GetNickname();
	if (ChatData->Sender.IsEmpty() || SenderABId.Get().GetAccelByteId().Equals(ChatData->Sender))
	{
		ChatData->Sender = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message.GetUserId().Get());
	}

	// Display chat message.
	W_ActiveChat->AppendChatMessage(ChatData);
}

void USessionChatWidget::SendChatMessage(const FText& MessageText)
{
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
		MessageText.ToString());
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

	if (!ensure(W_ActiveChat))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot get last chat messages. Chat widget component is not valid."));
		return;
	}

	// Get chat room id.
	FString ChatRoomId = SessionChatSubsystem->GetChatRoomIdBasedOnType(CurrentChatRoomType);

	// Abort if room id was not found.
	if (ChatRoomId.IsEmpty())
	{
		W_ActiveChat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	// Get last chat messages.
	TArray<TSharedRef<FChatMessage>> OutMessages;
	if (SessionChatSubsystem->GetLastChatMessages(
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		ChatRoomId,
		W_ActiveChat->GetMaxChatHistory(),
		OutMessages)) 
	{
		// Abort if last messages is empty.
		if (OutMessages.IsEmpty())
		{
			W_ActiveChat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
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
		W_ActiveChat->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}

void USessionChatWidget::OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful)
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
	UChatData* ChatData = NewObject<UChatData>();
	ChatData->Message = MsgBody;
	ChatData->bIsSenderLocal = true;
	W_ActiveChat->AppendChatMessage(ChatData);
}

void USessionChatWidget::OnChatRoomMessageReceived(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
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

	// Show the received chat message.
	AppendChatMessage(Message.Get());
}
