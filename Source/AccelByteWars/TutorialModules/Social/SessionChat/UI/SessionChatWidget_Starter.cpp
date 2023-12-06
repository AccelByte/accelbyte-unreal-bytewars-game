// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void USessionChatWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	SessionChatSubsystem = GameInstance->GetSubsystem<USessionChatSubsystem_Starter>();
	ensure(SessionChatSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void USessionChatWidget_Starter::NativeOnActivated()
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
	Edt_ChatMessage->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnChatMessageChanged);
	
	// Bind event to switch between chat message type.
	Btn_SessionChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::SESSION_V2);
	Btn_PartyChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::PARTY_V2);
	
	// TODO: Bind session chat events here.

	SwitchChatMessageType(CurrentChatRoomType);
}

void USessionChatWidget_Starter::NativeOnDeactivated()
{
	CurrentChatRoomType = EAccelByteChatRoomType::SESSION_V2;

	Edt_ChatMessage->OnTextCommitted.Clear();
	Edt_ChatMessage->OnTextChanged.Clear();
	Btn_Send->OnClicked().Clear();
	
	Btn_SessionChat->OnClicked().Clear();
	Btn_PartyChat->OnClicked().Clear();

	// TODO: Unbind session chat events here.

	Super::NativeOnDeactivated();
}

void USessionChatWidget_Starter::SetDefaultChatType(const EAccelByteChatRoomType ChatRoomType)
{
	CurrentChatRoomType = ChatRoomType;
}

void USessionChatWidget_Starter::SwitchChatMessageType(const EAccelByteChatRoomType ChatRoomType)
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

	// TODO: Show last chat message based on session chat type.
}

void USessionChatWidget_Starter::OnChatMessageChanged(const FText& Text)
{
	// Disable the send button if it is empty.
	Btn_Send->SetIsEnabled(!Text.IsEmpty());

	// Limit the chat message length.
	Edt_ChatMessage->SetText(FText::FromString(Edt_ChatMessage->GetText().ToString().Left(MaxMessageLength)));
}

#pragma region Module Session Chat Function Definitions

// TODO: Add your Module Session Chat function definitions here.

#pragma endregion