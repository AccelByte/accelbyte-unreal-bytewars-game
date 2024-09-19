// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Components/Prompt/GUICheat/GUICheatWidget.h"
#include "Core/UI/Components/Info/InfoWidget.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Input/CommonUIInputTypes.h"

void UAccelByteWarsBaseUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Register stacks
	Stacks.Add(EBaseUIStackType::Prompt, PromptStack);
	Stacks.Add(EBaseUIStackType::FTUE, FTUEStack);
	Stacks.Add(EBaseUIStackType::PushNotification, PushNotificationStack);
	Stacks.Add(EBaseUIStackType::Menu, MenuStack);
	Stacks.Add(EBaseUIStackType::InGameMenu, InGameMenuStack);
	Stacks.Add(EBaseUIStackType::InGameHUD, InGameHUDStack);

	if (!IsDesignTime())
	{
		// Configure stacks transition behavior.
		for (const EBaseUIStackType StackType : TEnumRange<EBaseUIStackType>())
		{
			TWeakObjectPtr<UCommonActivatableWidgetStack> Stack = MakeWeakObjectPtr<UCommonActivatableWidgetStack>(Stacks[StackType]);
			if (!ensure(Stack.IsValid())) 
			{
				continue;
			}

			Stack->OnTransitioningChanged.AddUObject(this, &UAccelByteWarsBaseUI::OnWidgetTransitionChanged);
			Stack->OnDisplayedWidgetChanged().AddUObject(this, &UAccelByteWarsBaseUI::OnDisplayedWidgetChanged, StackType);
		}
	
		// Display project info widget to the target widgets.
		for (TSubclassOf<UAccelByteWarsActivatableWidget> TargetWidget : ProjectInfoTargetWidgets)
		{
			if (!TargetWidget.Get() || !TargetWidget.GetDefaultObject())
			{
				continue;
			}

			TargetWidget.GetDefaultObject()->OnActivated().RemoveAll(this);
			TargetWidget.GetDefaultObject()->OnDeactivated().RemoveAll(this);

			TargetWidget.GetDefaultObject()->OnActivated().AddUObject(this, &ThisClass::ToggleProjectInfoWidget, true);
			TargetWidget.GetDefaultObject()->OnDeactivated().AddUObject(this, &ThisClass::ToggleProjectInfoWidget, false);
		}

		// GUI Cheat setup
		if (AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_GUI_CHEAT, FLAG_GUI_CHEAT_SECTION, true))
		{
			// Bind key to show GUI Cheat
			OpenGUICheatHandle.Unregister();
			FBindUIActionArgs GUICheatBinding(
				OpenGUICheatInputActionData,
				false,
				FSimpleDelegate::CreateUObject(this, &ThisClass::ToggleGUICheat));
			GUICheatBinding.bIsPersistent = true;
			OpenGUICheatHandle = RegisterUIActionBinding(GUICheatBinding);
		}
	}
}

void UAccelByteWarsBaseUI::ClearWidgets()
{
	FTUEStack->ClearWidgets();
	MenuStack->ClearWidgets();
	InGameMenuStack->ClearWidgets();
	InGameHUDStack->ClearWidgets();
	PushNotificationStack->ClearWidgets();

	bStacksCleared = true;
}

void UAccelByteWarsBaseUI::ResetWidget()
{
	bStacksCleared = false;
}

void UAccelByteWarsBaseUI::ToggleBackgroundBlur(const bool bShow) const
{
	BackgroundBlur->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UAccelByteWarsBaseUI::ToggleProjectInfoWidget(const bool bShow) const
{
	W_ProjectInfo->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bShow) 
	{
		W_ProjectInfo->RefreshUI();
	}
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

	const UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget(false);
	if (!BaseUIWidget) 
	{
		return nullptr;
	}

	return BaseUIWidget->Stacks[TargetStack]->GetActiveWidget();
}

TArray<UCommonActivatableWidget*> UAccelByteWarsBaseUI::GetAllWidgetsBelowStacks(const EBaseUIStackType CurrentStack)
{
	// Get all widgets below given stack.
	TArray<UCommonActivatableWidget*> Result;
	for (const EBaseUIStackType StackType : TEnumRange<EBaseUIStackType>())
	{
		if (StackType <= CurrentStack)
		{
			continue;
		}

		TWeakObjectPtr<UCommonActivatableWidget> ActiveStack = MakeWeakObjectPtr<UCommonActivatableWidget>(Stacks[StackType]->GetActiveWidget());
		if (ActiveStack.IsValid())
		{
			Result.Add(ActiveStack.Get());
		}
	}

	return Result;
}

EBaseUIStackType UAccelByteWarsBaseUI::GetTopMostActiveStack()
{
	EBaseUIStackType TopMostStackType = EBaseUIStackType::Prompt;
	
	for (const EBaseUIStackType StackType : TEnumRange<EBaseUIStackType>())
	{
		TopMostStackType = StackType;
		if (Stacks[StackType]->GetActiveWidget())
		{
			break;
		}
	}

	return TopMostStackType;
}

void UAccelByteWarsBaseUI::AddGUICheatEntry(UGUICheatWidgetEntry* Entry)
{
	if (!W_GUICheat)
	{
		return;
	}

	W_GUICheat->AddEntry(Entry);
}

void UAccelByteWarsBaseUI::RemoveGUICheatEntry(UGUICheatWidgetEntry* Entry)
{
	if (!W_GUICheat || !Entry)
	{
		return;
	}

	W_GUICheat->RemoveEntry(Entry);
}

void UAccelByteWarsBaseUI::RemoveGUICheatEntries(UTutorialModuleDataAsset* TutorialModuleDataAsset)
{
	if (!W_GUICheat || !TutorialModuleDataAsset)
	{
		return;
	}

	W_GUICheat->RemoveEntries(TutorialModuleDataAsset);
}

UAccelByteWarsActivatableWidget* UAccelByteWarsBaseUI::PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	return PushWidgetToStack(TargetStack, WidgetClass, [](UCommonActivatableWidget&) {});
}

UAccelByteWarsActivatableWidget* UAccelByteWarsBaseUI::PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, TFunctionRef<void(UAccelByteWarsActivatableWidget&)> InitFunc)
{
	UCommonActivatableWidgetStack* Stack = Stacks[TargetStack];
	ensure(Stack);

	// safeguard to not add a widget when the world is tearing down
	if (GetWorld()->bIsTearingDown)
	{
		return nullptr;
	}

	TWeakObjectPtr<UAccelByteWarsActivatableWidget> NewWidget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(Cast<UAccelByteWarsActivatableWidget>(Stack->AddWidget(WidgetClass, InitFunc)));
	if (NewWidget.IsValid())
	{
		NewWidget->SetWidgetStackType(TargetStack);
	}

	return NewWidget.Get();
}

UPushNotificationWidget* UAccelByteWarsBaseUI::GetPushNotificationWidget()
{
	if (bStacksCleared)
	{
		return nullptr;
	}

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

void UAccelByteWarsBaseUI::SetFTUEDialogueWidget(UFTUEDialogueWidget* InFTUEDialogueWidget)
{
	W_FTUEDialogue = InFTUEDialogueWidget;
}

void UAccelByteWarsBaseUI::OnWidgetTransitionChanged(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	// Set auto focus to top most stack widget.
	FocusOnTopMostStack();
}

void UAccelByteWarsBaseUI::OnDisplayedWidgetChanged(UCommonActivatableWidget* Widget, const EBaseUIStackType StackType)
{
	TWeakObjectPtr<UAccelByteWarsActivatableWidget> NewWidget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(Cast<UAccelByteWarsActivatableWidget>(Widget));
	if (NewWidget.IsValid())
	{
		// Moving the camera location only allowed on Menu stack.
		NewWidget->AllowMoveCamera(StackType == EBaseUIStackType::Menu);
	}

	// For game menu, to make the UI visible above the level, toggle the background blur.
	if (StackType == EBaseUIStackType::Menu || StackType == EBaseUIStackType::InGameMenu)
	{
		ToggleBackgroundBlur(NewWidget.IsValid());
	}
}

void UAccelByteWarsBaseUI::FocusOnTopMostStack()
{
	// Remove focus override
	if (FocusOverride == W_GUICheat && W_GUICheat->IsEntriesEmpty())
	{
		RemoveFocusOverride();
	}

	bool bIsTopMostStackFound = false;
	for (const EBaseUIStackType StackType : TEnumRange<EBaseUIStackType>())
	{
		TWeakObjectPtr<UCommonActivatableWidgetStack> Stack = MakeWeakObjectPtr<UCommonActivatableWidgetStack>(Stacks[StackType]);
		if (!Stack.IsValid()) 
		{
			continue;
		}

		if (!bIsTopMostStackFound && Stack->GetActiveWidget() && (!FocusOverride || FocusOverride == Stack))
		{
			Stack->SetVisibility(ESlateVisibility::Visible);
			bIsTopMostStackFound = true;
			continue;
		}

		Stack->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UAccelByteWarsBaseUI::OverrideFocus(UWidget* Target)
{
	FocusOverride = Target;
	FocusOnTopMostStack();
}

void UAccelByteWarsBaseUI::RemoveFocusOverride()
{
	FocusOverride = nullptr;
	FocusOnTopMostStack();
}

void UAccelByteWarsBaseUI::ToggleGUICheat()
{
	if (W_GUICheat->IsActivated())
	{
		W_GUICheat->DeactivateWidget();
		RemoveFocusOverride();
	}
	else if (!W_GUICheat->IsEntriesEmpty())
	{
		W_GUICheat->ActivateWidget();
		OverrideFocus(W_GUICheat);
	}
}
