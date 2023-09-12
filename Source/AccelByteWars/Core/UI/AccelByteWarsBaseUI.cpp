// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/System/AccelByteWarsGameInstance.h"

void UAccelByteWarsBaseUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Register stacks
	Stacks.Add(EBaseUIStackType::Menu, MenuStack);
	Stacks.Add(EBaseUIStackType::Prompt, PromptStack);
	Stacks.Add(EBaseUIStackType::InGameMenu, InGameMenuStack);
	Stacks.Add(EBaseUIStackType::InGameHUD, InGameHUDStack);

	if (!IsDesignTime())
	{
		UCommonActivatableWidgetStack* Stack;
		for (int i = EBaseUIStackType::Prompt; i != EBaseUIStackType::InGameHUD; i++)
		{
			Stack = Stacks[static_cast<EBaseUIStackType>(i)];
			ensure(Stack);

			Stack->OnTransitioningChanged.AddUObject(this, &UAccelByteWarsBaseUI::OnWidgetTransitionChanged);
		}
	}
}

void UAccelByteWarsBaseUI::ToggleBackgroundBlur(const bool bShow) const
{
	BackgroundBlur->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

UCommonActivatableWidget* UAccelByteWarsBaseUI::GetActiveWidgetOfStack(const EBaseUIStackType TargetStack, const UObject* Context)
{
	if (!Context || !Context->GetWorld())
	{
		return nullptr;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(Context->GetWorld()->GetGameInstance());
	if (!GameInstance) 
	{
		return nullptr;
	}

	const UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget) 
	{
		return nullptr;
	}

	return BaseUIWidget->Stacks[TargetStack]->GetActiveWidget();
}

UAccelByteWarsActivatableWidget* UAccelByteWarsBaseUI::PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	return PushWidgetToStack(TargetStack, WidgetClass, [](UCommonActivatableWidget&) {});
}

UAccelByteWarsActivatableWidget* UAccelByteWarsBaseUI::PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, TFunctionRef<void(UAccelByteWarsActivatableWidget&)> InitFunc)
{
	UCommonActivatableWidgetStack* Stack = Stacks[TargetStack];
	ensure(Stack);

	return Cast<UAccelByteWarsActivatableWidget>(Stack->AddWidget(WidgetClass, InitFunc));
}

UPushNotificationWidget* UAccelByteWarsBaseUI::GetPushNotificationWidget()
{
	UPushNotificationWidget* PushNotificationWidget = Cast<UPushNotificationWidget>(PushNotificationStack->GetActiveWidget());

	if (!PushNotificationWidget)
	{
		PushNotificationWidget = Cast<UPushNotificationWidget>(PushNotificationStack->AddWidget(DefaultPushNotificationWidgetClass.Get()));
	}

	return PushNotificationWidget;
}

UFTUEDialogueWidget* UAccelByteWarsBaseUI::GetFTUEDialogueWidget()
{
	return W_FTUEDialogue;
}

void UAccelByteWarsBaseUI::OnWidgetTransitionChanged(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	// Set auto focus to top most stack widget.
	EBaseUIStackType StackType;
	UCommonActivatableWidgetStack* Stack;
	bool bIsTopMostStackFound = false;

	for (int i = EBaseUIStackType::Prompt; i <= EBaseUIStackType::InGameHUD; i++)
	{
		StackType = static_cast<EBaseUIStackType>(i);
		Stack = Stacks[StackType];

		if (!bIsTopMostStackFound && Stack->GetActiveWidget())
		{
			Stack->SetVisibility(ESlateVisibility::Visible);
			bIsTopMostStackFound = true;
			continue;
		}

		Stack->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}