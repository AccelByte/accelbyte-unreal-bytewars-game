// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ByteWarsCore/UI/AccelByteWarsBaseUI.h"

UAccelByteWarsActivatableWidget* UAccelByteWarsBaseUI::PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	ensure(MenuStack != nullptr);
	ensure(PromptStack != nullptr);
	ensure(InGameMenuStack != nullptr);
	ensure(InGameHUDStack != nullptr);

	UAccelByteWarsActivatableWidget* NewWidget = nullptr;
	switch (TargetStack) 
	{
		case EBaseUIStackType::Menu:
			NewWidget = Cast<UAccelByteWarsActivatableWidget>(MenuStack->AddWidget(WidgetClass));
			break;
		case EBaseUIStackType::Prompt:
			NewWidget = Cast<UAccelByteWarsActivatableWidget>(PromptStack->AddWidget(WidgetClass));
			break;
		case EBaseUIStackType::InGameMenu:
			NewWidget = Cast<UAccelByteWarsActivatableWidget>(InGameMenuStack->AddWidget(WidgetClass));
			break;
		case EBaseUIStackType::InGameHUD:
			NewWidget = Cast<UAccelByteWarsActivatableWidget>(InGameHUDStack->AddWidget(WidgetClass));
			break;
	}

	return NewWidget;
}
