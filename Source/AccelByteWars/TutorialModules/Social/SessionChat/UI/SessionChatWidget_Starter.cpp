// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Social/ChatEssentials/UI/ChatWidget.h"

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

	// Reset chat widget
	W_GameSessionChat->ClearChatMessages();
	W_GameSessionChat->ClearInput();
	W_PartyChat->ClearChatMessages();
	
	// Bind event to switch between chat message type.
	Btn_SessionChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::SESSION_V2);
	Btn_PartyChat->OnClicked().AddUObject(this, &ThisClass::SwitchChatMessageType, EAccelByteChatRoomType::PARTY_V2);

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	
	// TODO: Bind session chat events here.

	SwitchChatMessageType(CurrentChatRoomType);
}

void USessionChatWidget_Starter::NativeOnDeactivated()
{
	CurrentChatRoomType = EAccelByteChatRoomType::SESSION_V2;
	
	Btn_SessionChat->OnClicked().Clear();
	Btn_PartyChat->OnClicked().Clear();
	Btn_Back->OnClicked().RemoveAll(this);

	// TODO: Unbind session chat events here.

	Super::NativeOnDeactivated();
}

UWidget* USessionChatWidget_Starter::NativeGetDesiredFocusTarget() const
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
		Ws_ChatMessageType->SetActiveWidget(W_GameSessionChatOuter);
		W_ActiveChat = W_GameSessionChat;
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		Ws_ChatMessageType->SetActiveWidget(W_PartyChatOuter);
		W_ActiveChat = W_PartyChat;
		break;
	}

	CurrentChatRoomType = ChatRoomType;

	// TODO: Show last chat message based on session chat type.
}

#pragma region Module Session Chat Function Definitions

// TODO: Add your Module Session Chat function definitions here.

#pragma endregion