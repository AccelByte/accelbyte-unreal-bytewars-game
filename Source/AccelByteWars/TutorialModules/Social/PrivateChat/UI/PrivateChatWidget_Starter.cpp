// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "CommonButtonBase.h"
#include "Social/ChatEssentials/UI/ChatWidget.h"

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

	// Reset chat widget
	W_Chat->ClearChatMessages();
	W_Chat->ClearInput();
	
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	
	// TODO: Bind private chat events here.

	// TODO: Get and display private chat history here.
}

void UPrivateChatWidget_Starter::NativeOnDeactivated()
{
	PrivateChatRecipientUserId = nullptr;

	Btn_Back->OnClicked().RemoveAll(this);

	// TODO: Unbind private chat events here.

	Super::NativeOnDeactivated();
}

UWidget* UPrivateChatWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return W_Chat;
}

void UPrivateChatWidget_Starter::SetPrivateChatRecipient(FUniqueNetIdPtr RecipientUserId)
{
	PrivateChatRecipientUserId = RecipientUserId;
}

#pragma region Module Private Chat Function Definitions

// TODO: Add your Module Private Chat function definitions here.

#pragma endregion