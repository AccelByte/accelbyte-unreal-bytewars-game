// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

void UPrivateChatWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PrivateChatSubsystem = GameInstance->GetSubsystem<UPrivateChatSubsystem_Starter>();
	ensure(PrivateChatSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UPrivateChatWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset private chat messages.
	Lv_ChatMessage->ClearListItems();

	// Reset chat box.
	Edt_ChatMessage->SetText(FText::GetEmpty());
	Edt_ChatMessage->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnChatMessageChanged);
	
	// TODO: Bind private chat events here.

	// TODO: Get and display private chat history here.
}

void UPrivateChatWidget_Starter::NativeOnDeactivated()
{
	PrivateChatRecipientUserId = nullptr;

	Edt_ChatMessage->OnTextCommitted.Clear();
	Edt_ChatMessage->OnTextChanged.Clear();
	Btn_Send->OnClicked().Clear();

	// TODO: Unbind private chat events here.

	Super::NativeOnDeactivated();
}

void UPrivateChatWidget_Starter::SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId)
{
	PrivateChatRecipientUserId = RecipientUserId;
}

void UPrivateChatWidget_Starter::OnChatMessageChanged(const FText& Text)
{
	// Disable the send button if it is empty.
	Btn_Send->SetIsEnabled(!Text.IsEmpty());

	// Limit the chat message length.
	Edt_ChatMessage->SetText(FText::FromString(Edt_ChatMessage->GetText().ToString().Left(MaxMessageLength)));
}

#pragma region Module Private Chat Function Definitions

// TODO: Add your Module Private Chat function definitions here.

#pragma endregion