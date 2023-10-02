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
		DialoguesOrigin.AddUnique(Dialogues[i]);
	}

	ValidateDialogues();
}

bool UFTUEDialogueWidget::RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	int32 Removed = DialoguesOrigin.RemoveAll([WidgetClass](const FFTUEDialogueModel* Temp)
	{
		return !Temp || Temp->TargetWidgetClasses.Contains(WidgetClass);
	});

	ValidateDialogues();

	return Removed > 0;
}

void UFTUEDialogueWidget::ShowDialogues(bool bFirstTime)
{
	if (W_FTUEDialogue->IsVisible())
	{
		return;
	}

	ValidateDialogues();
	
	// Check whether should abort if all dialogues are already shown.
	if (bFirstTime)
	{
		// Reset is already shown dialogue statuses if always show FTUE.
		if (GameInstance && GameInstance->GetFTUEAlwaysOnSetting())
		{
			TArray<FFTUEDialogueModel*> Instigators = DialoguesInternal.FilterByPredicate([](const FFTUEDialogueModel* Temp)
			{
				return Temp && Temp->bIsInstigator;
			});
			for (auto Instigator : Instigators)
			{
				if (Instigator->Group)
				{
					Instigator->Group->SetAlreadyShown(false);
				}
			}

			DialoguesInternal.RemoveAll([](const FFTUEDialogueModel* Temp)
			{
				return !Temp || Temp->bIsAlreadyShown;
			});
		}

		// Abort if all dialogues is already shown.
		if (IsAllDialoguesAlreadyShown()) 
		{
			return;
		}
	}

	// Abort if after validation, the dialogues turns out to be empty.
	if (DialoguesInternal.IsEmpty()) 
	{
		return;
	}

	// Initialize FTUE.
	ClearHighlightedWidget();
	DialogueIndex = 0;
	CachedLastDialogue = nullptr;
	DialoguesInternal.Sort();

	// Show FTUE.
	if (InitializeDialogue(DialoguesInternal[DialogueIndex]))
	{
		W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);
		TryToggleHelpDev(false);
	}
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
	DialogueIndex = INDEX_NONE;
	CachedLastDialogue = nullptr;

	// Close the FTUE
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
	TryToggleHelpDev(true);
}

void UFTUEDialogueWidget::PauseDialogues()
{
	// Simple hide FTUE to pause dialogues.
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
	TryToggleHelpDev(false);
}

void UFTUEDialogueWidget::ResumeDialogues()
{
	// If dialogue index is out of range, initialize.
	if (DialogueIndex < 0 || DialogueIndex > DialoguesInternal.Num() - 1)
	{
		ShowDialogues(true);
		return;
	}

	// Only able to resume dialogues if the dialogues are not already shown.
	if (IsAllDialoguesAlreadyShown()) 
	{
		return;
	}

	// Show FTUE to resume dialogues.
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);
	if (CachedLastDialogue)
	{
		W_FTUEInterupter->SetVisibility(
			CachedLastDialogue->bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
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
	if (!InitializeDialogue(DialoguesInternal[--DialogueIndex]))
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

	if (DialogueIndex >= DialoguesInternal.Num() - 1)
	{
		DialogueIndex = DialoguesInternal.Num() - 1;
		CloseDialogues();
		return;
	}

	// Try to init dialogue. If fails, fallback.
	if (!InitializeDialogue(DialoguesInternal[++DialogueIndex]))
	{
		NextDialogue();
	}
}

void UFTUEDialogueWidget::TryToggleHelpDev(bool bShow)
{
	// Always hide the help button if the FTUE is visible or there is no dialogues.
	if (W_FTUEDialogue->IsVisible() || DialoguesInternal.IsEmpty())
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
		// Look for widget to highlight.
		const FString WidgetToHighlightStr = Dialogue->TargetWidgetNameToHighlight;
		UE_LOG_FTUEDIALOGUEWIDGET(Log, TEXT("Widget to highlight: %s"), *WidgetToHighlightStr);

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, Dialogue->TargetWidgetClassToHighlight.Get(), false);
		UE_LOG_FTUEDIALOGUEWIDGET(Log, TEXT("Found potential widgets to highlight: %d"), FoundWidgets.Num());

		// Eliminate invalid found widgets.
		FoundWidgets.RemoveAll([WidgetToHighlightStr](const UUserWidget* Temp)
		{
			return !Temp || !Temp->GetName().Equals(WidgetToHighlightStr) || !Temp->IsVisible();
		});
		UE_LOG_FTUEDIALOGUEWIDGET(Log, TEXT("Potential widgets to highlight after validation: %d"), FoundWidgets.Num());

		// Try to highlight widget.
		for (auto& FoundWidget : FoundWidgets)
		{
			// Skip if invalid.
			if (!FoundWidget)
			{
				UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Highlighted widget is found but it is not valid."));
				continue;
			}
			if (!FoundWidget->IsVisible())
			{
				UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Highlighted widget is found but it is invisible."));
				continue;
			}

			// Widget found, highlight widget.
			if (FoundWidget->GetName().Equals(WidgetToHighlightStr, ESearchCase::CaseSensitive))
			{
				// Highlight widget.
				IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(FoundWidget);
				if (WidgetInterface)
				{
					CachedHighlightedWidget = FoundWidget;
					WidgetInterface->Execute_ToggleHighlight(FoundWidget, true);

					UE_LOG_FTUEDIALOGUEWIDGET(Log, TEXT("Highlighted widget found."));
				}
				else 
				{
					UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Highlighted widget is found but it is not implementing widget highlighting interface."));
				}

				break;
			}
		}

		// Highlighted widget is not found, abort.
		if (!CachedHighlightedWidget)
		{
			UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Cannot highlight widget. Skipping FTUE dialogue, highlighted widget is not found or invisible."));
			return false;
		}
	}

	// Check if interrupting.
	W_FTUEInterupter->SetVisibility(
		Dialogue->bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Set navigation buttons.
	Btn_Next->SetButtonText(
		(bIsAbleToNavigate && DialogueIndex < DialoguesInternal.Num() - 1) ?
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

	CachedLastDialogue = Dialogue;

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
	Button->SetButtonText(ButtonModel.GetFormattedButtonText());

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
	DialoguesOrigin.RemoveAll([](const FFTUEDialogueModel* Temp)
	{
		return !Temp || 
			!Temp->OwnerTutorialModule || 
			!Temp->OwnerTutorialModule->IsActiveAndDependenciesChecked() ||
			(Temp->OnValidateDelegate.IsBound() && !Temp->OnValidateDelegate.Execute());
	});

	DialoguesInternal = DialoguesOrigin;
	TryToggleHelpDev(!DialoguesInternal.IsEmpty());
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

bool UFTUEDialogueWidget::IsAllDialoguesAlreadyShown()
{
	TArray<FFTUEDialogueModel*> AlreadyShown = DialoguesInternal.FilterByPredicate([](const FFTUEDialogueModel* Temp)
	{
		return Temp && Temp->bIsAlreadyShown;
	});

	return AlreadyShown.Num() == DialoguesInternal.Num();
}

#undef LOCTEXT_NAMESPACE