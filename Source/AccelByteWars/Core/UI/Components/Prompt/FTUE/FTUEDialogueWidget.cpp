// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/TextBlock.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

DEFINE_LOG_CATEGORY(LogFTUEDialogueWidget);

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFTUEDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());

	Btn_Open->OnClicked().Clear();
	Btn_Open->OnClicked().AddUObject(this, &ThisClass::ShowDialogues, false);

	Btn_Prev->OnClicked().Clear();
	Btn_Prev->OnClicked().AddUObject(this, &ThisClass::PrevDialogue);

	Btn_Next->OnClicked().Clear();
	Btn_Next->OnClicked().AddUObject(this, &ThisClass::NextDialogue);

	// On initialize, close the FTUE.
	CloseDialogues();
}

void UFTUEDialogueWidget::AddDialogues(const TArray<FFTUEDialogueModel*>& Dialogues)
{
	// Add dialogues to cache.
	for (int i = 0; i < Dialogues.Num(); i++) 
	{
		CachedDialogues.Add(Dialogues[i]);
	}

	ValidateDialogues();
}

bool UFTUEDialogueWidget::RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	int32 Removed = CachedDialogues.RemoveAll([WidgetClass](const FFTUEDialogueModel* Temp)
	{
		return !Temp || Temp->TargetWidgetClasses.Contains(WidgetClass);
	});

	ValidateDialogues();

	return Removed > 0;
}

void UFTUEDialogueWidget::ShowDialogues(bool bFirstTime)
{
	if (CachedDialogues.IsEmpty() || W_FTUEDialogue->IsVisible())
	{
		return;
	}

	ValidateDialogues();

	// Check whether should remove dialogues that already shown.
	if (bFirstTime && !(GameInstance && GameInstance->GetFTUEAlwaysOnSetting())) 
	{
		// Remove all already shown dialogues.
		TArray<FFTUEDialogueModel*> AlreadyShown = CachedDialogues.FilterByPredicate([](const FFTUEDialogueModel* Temp)
		{
			return Temp && Temp->bIsAlreadyShown;
		});

		// Abort if all dialogues is already show.
		if (AlreadyShown.Num() == CachedDialogues.Num())
		{
			return;
		}
	}

	// Initialize FTUE.
	ClearHighlightedWidget();
	DialogueIndex = 0;
	CachedLastDialogue = nullptr;
	CachedDialogues.Sort();
	InitializeDialogue(CachedDialogues[DialogueIndex]);

	// Show FTUE.
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);
	TryToggleHelpDev(false);
}

void UFTUEDialogueWidget::CloseDialogues()
{
	if (!W_FTUEDialogue->IsVisible()) 
	{
		return;
	}

	// Tear down FTUE.
	ClearHighlightedWidget();
	DeinitializeLastDialogue();
	ValidateDialogues();
	CachedLastDialogue = nullptr;

	// Close the FTUE
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
	TryToggleHelpDev(true);
}

void UFTUEDialogueWidget::PauseDialogues()
{
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	TryToggleHelpDev(false);
}

void UFTUEDialogueWidget::ResumeDialogues()
{
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);
	TryToggleHelpDev(false);
}

void UFTUEDialogueWidget::PrevDialogue()
{
	if (!W_FTUEDialogue->IsVisible()) 
	{
		return;
	}

	if (DialogueIndex <= 0)
	{
		DialogueIndex = 0;
		return;
	}

	// Try to init dialogue. If fails, fallback.
	if (!InitializeDialogue(CachedDialogues[--DialogueIndex])) 
	{
		if (DialogueIndex <= 0) 
		{
			CloseDialogues();
		}
		else 
		{
			PrevDialogue();
		}
	}
}

void UFTUEDialogueWidget::NextDialogue()
{
	if (!W_FTUEDialogue->IsVisible())
	{
		return;
	}

	if (DialogueIndex >= CachedDialogues.Num() - 1)
	{
		DialogueIndex = CachedDialogues.Num() - 1;
		CloseDialogues();
		return;
	}

	// Try to init dialogue. If fails, fallback.
	if (!InitializeDialogue(CachedDialogues[++DialogueIndex])) 
	{
		NextDialogue();
	}
}

void UFTUEDialogueWidget::TryToggleHelpDev(bool bShow)
{
	// Always hide the help button if the FTUE is visible or there is no dialogues.
	if (W_FTUEDialogue->IsVisible() || CachedDialogues.IsEmpty())
	{
		Btn_Open->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Set visibility based on the given toggle status.
	Btn_Open->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

bool UFTUEDialogueWidget::InitializeDialogue(FFTUEDialogueModel* Dialogue)
{
	if (!Dialogue)
	{
		return false;
	}

	// Check for highlighted widget.
	ClearHighlightedWidget();
	if (Dialogue->bHighlightWidget)
	{
		const FString WidgetToHighlightStr = Dialogue->TargetWidgetNameToHighlight;
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, Dialogue->TargetWidgetClassToHighlight.Get(), false);

		for (auto& FoundWidget : FoundWidgets)
		{
			if (!FoundWidget)
			{
				continue;
			}

			// Highlight widget if visible.
			if (FoundWidget->GetName().Equals(WidgetToHighlightStr, ESearchCase::CaseSensitive))
			{
				// Check whether the widget is visible or not. 
				// TODO: There is wierd behavior in Unreal where is visible does not count the parent widget.
				if (!FoundWidget->IsVisible()) 
				{
					UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Cannot highlight widget. Skipping FTUE dialogue, highlighted widget is not found or invisible."));
					return false;
				}

				IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(FoundWidget);
				if (WidgetInterface)
				{
					CachedHighlightedWidget = FoundWidget;
					WidgetInterface->Execute_ToggleHighlight(FoundWidget, true);
				}
				break;
			}
		}
	}

	// Check if interrupting.
	W_FTUEInterupter->SetVisibility(
		Dialogue->bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Set navigation buttons.
	Btn_Next->SetButtonText(
		(bIsAbleToNavigate && DialogueIndex < CachedDialogues.Num() - 1) ?
		LOCTEXT("Next", "Next") :
		LOCTEXT("X", "X"));
	Btn_Prev->SetVisibility(
		(bIsAbleToNavigate && DialogueIndex > 0) ?
		ESlateVisibility::Visible :
		ESlateVisibility::Collapsed);

	// Set dialogue message.
	Txt_Message->SetText(Dialogue->GetFormattedMessage());

	// Setup the action buttons.
	Btn_Action1->SetVisibility(ESlateVisibility::Collapsed);
	Btn_Action2->SetVisibility(ESlateVisibility::Collapsed);
	switch (Dialogue->ButtonType)
	{
	case FFTUEDialogueButtonType::TWO_BUTTONS:
		InitializeActionButton(Btn_Action2, Dialogue->Button2);
	case FFTUEDialogueButtonType::ONE_BUTTON:
		InitializeActionButton(Btn_Action1, Dialogue->Button1);
		break;
	}

	// Set dialogue position.
	if (UCanvasPanelSlot* WidgetSlot = Cast<UCanvasPanelSlot>(W_FTUEDialogue->Slot)) 
	{
		WidgetSlot->SetAutoSize(true);
		WidgetSlot->SetAnchors(FAnchors(Dialogue->GetAnchor().X, Dialogue->GetAnchor().Y));
		WidgetSlot->SetAlignment(Dialogue->GetAnchor());
		WidgetSlot->SetPosition(Dialogue->Position);
	}
	
	// Execute last dialogue's on-deactivate event.
	DeinitializeLastDialogue();

	// Execute current dialogue's on-activated event.
	if (Dialogue->OnActivateDelegate.IsBound()) 
	{
		Dialogue->OnActivateDelegate.Broadcast();
	}

	/* Reset mark already shown.
	 * This way, all siblings in the dialogue group requires 
	 * to be shown until the last dialogue in the group.
	 * Later once it is done, the group will be marked as shown. */
	if (Dialogue->bIsAlreadyShown && Dialogue->Group)
	{
		Dialogue->Group->SetAlreadyShown(false);
	}

	CachedLastDialogue = CachedDialogues[DialogueIndex];

	return true;
}

void UFTUEDialogueWidget::InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel)
{
	if (!Button) 
	{
		return;
	}

	Button->OnClicked().Clear();

	Button->SetVisibility(ESlateVisibility::Visible);
	Button->SetButtonText(ButtonModel.ButtonText);

	// Bind action to open hyperlink.
	if (ButtonModel.ButtonActionType == EFTUEDialogueButtonActionType::HYPERLINK_BUTTON)
	{
		const FString URL = ButtonModel.GetFormattedURL();
		Button->OnClicked().AddWeakLambda(this, [URL]()
		{
			if (URL.IsEmpty())
			{
				UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Cannot open hyperlink. Dialogue button's hyperlink is empty."));
				return;
			}

			FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
		});
	}
	// Bind action to execute custom function.
	else if (ButtonModel.ButtonActionType == EFTUEDialogueButtonActionType::ACTION_BUTTON)
	{
		Button->OnClicked().AddWeakLambda(this, [ButtonModel]()
		{
			if (!ButtonModel.ButtonActionDelegate.IsBound()) 
			{
				UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Cannot execute custom action. Dialogue button is not bound to any event."));
				return;
			}

			ButtonModel.ButtonActionDelegate.Broadcast();
		});
	}
}

void UFTUEDialogueWidget::ValidateDialogues()
{
	// Remove invalid dialogues.
	CachedDialogues.RemoveAll([](const FFTUEDialogueModel* Temp)
	{
		return !Temp || 
			!Temp->OwnerTutorialModule || 
			!Temp->OwnerTutorialModule->IsActiveAndDependenciesChecked() ||
			(Temp->OnValidateDelegate.IsBound() && !Temp->OnValidateDelegate.Execute());
	});

	TryToggleHelpDev(!CachedDialogues.IsEmpty());
}

void UFTUEDialogueWidget::DeinitializeLastDialogue()
{
	if (!CachedLastDialogue) 
	{
		return;
	}

	// Mark last dialogue as shown.
	if (!CachedLastDialogue->bIsAlreadyShown && CachedLastDialogue->bIsTerminator && CachedLastDialogue->Group)
	{
		CachedLastDialogue->Group->SetAlreadyShown(true);
		if (CachedLastDialogue->Group->OwnerTutorialModule)
		{
			CachedLastDialogue->Group->OwnerTutorialModule->SaveAttributesToLocal();
		}
	}

	// Execute last dialogue's on-deactivate event.
	if (CachedLastDialogue->OnDeactivateDelegate.IsBound()) 
	{
		CachedLastDialogue->OnDeactivateDelegate.Broadcast();
	}
}

void UFTUEDialogueWidget::ClearHighlightedWidget()
{
	if (CachedHighlightedWidget)
	{
		IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(CachedHighlightedWidget);
		if (WidgetInterface)
		{
			WidgetInterface->Execute_ToggleHighlight(CachedHighlightedWidget, false);
		}
	}

	CachedHighlightedWidget = nullptr;
}

#undef LOCTEXT_NAMESPACE