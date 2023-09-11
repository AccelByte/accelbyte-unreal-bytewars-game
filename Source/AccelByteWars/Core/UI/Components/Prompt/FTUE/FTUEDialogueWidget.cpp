// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/TextBlock.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFTUEDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Open->OnClicked().Clear();
	Btn_Open->OnClicked().AddUObject(this, &ThisClass::ShowDialogues);

	Btn_Prev->OnClicked().Clear();
	Btn_Prev->OnClicked().AddUObject(this, &ThisClass::PrevDialogue);

	Btn_Next->OnClicked().Clear();
	Btn_Next->OnClicked().AddUObject(this, &ThisClass::NextDialogue);

	// TODO: Might prefer to remove the widget instead of changing its visibility.
	// On initialize, close the FTUE.
	Btn_Open->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
}

void UFTUEDialogueWidget::AddDialogues(TArray<FFTUEDialogueModel> Dialogues)
{
	CachedDialogues.Append(Dialogues);

	Btn_Open->SetVisibility(!CachedDialogues.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFTUEDialogueWidget::RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	CachedDialogues.RemoveAll([WidgetClass](const FFTUEDialogueModel Temp)
	{
		return Temp.TargetWidgetClasses.Contains(WidgetClass);
	});

	Btn_Open->SetVisibility(!CachedDialogues.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFTUEDialogueWidget::ShowDialogues()
{
	DialogueIndex = 0;

	if (CachedDialogues.IsEmpty()) 
	{
		return;
	}

	ClearHighlightedWidget();

	W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);

	CachedDialogues.Sort();
	InitializeDialogue(CachedDialogues[DialogueIndex]);
}

void UFTUEDialogueWidget::CloseDialogues()
{
	ClearHighlightedWidget();

	// TODO: Might prefer to remove the widget instead of changing its visibility.
	// Close the FTUE
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
}

void UFTUEDialogueWidget::PrevDialogue()
{
	if (DialogueIndex <= 0)
	{
		DialogueIndex = 0;
		return;
	}

	InitializeDialogue(CachedDialogues[--DialogueIndex]);
}

void UFTUEDialogueWidget::NextDialogue()
{
	if (DialogueIndex >= CachedDialogues.Num() - 1)
	{
		DialogueIndex = CachedDialogues.Num() - 1;
		CloseDialogues();
		return;
	}

	InitializeDialogue(CachedDialogues[++DialogueIndex]);
}

void UFTUEDialogueWidget::InitializeDialogue(const FFTUEDialogueModel& Dialogue)
{
	// Check for highlighted widget.
	ClearHighlightedWidget();
	if (Dialogue.bHighlightWidget)
	{
		const FString WidgetToHighlightStr = Dialogue.TargetWidgetNameToHighlight;
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, Dialogue.TargetWidgetClassToHighlight.Get(), false);

		for (auto& FoundWidget : FoundWidgets)
		{
			if (!FoundWidget)
			{
				continue;
			}

			// Highlight widget.
			// TODO: check whether the widget is visible or not (there is wierd behavior in Unreal).
			if (FoundWidget->GetName().Equals(WidgetToHighlightStr, ESearchCase::CaseSensitive))
			{
				IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(FoundWidget);
				if (WidgetInterface)
				{
					CachedHighlightedWidget = FoundWidget;
					WidgetInterface->Execute_ToggleHighlight(FoundWidget, true);
				}
				break;
			}
		}
	
		// Skip the dialogue if the highlighted widget is not found or is invisible.
		// TODO: check whether the widget is visible or not (there is wierd behavior in Unreal).
		if (!CachedHighlightedWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot highlight widget. Skipping FTUE dialogue, highlighted widget is not found or invisible."));
			NextDialogue();
			return;
		}
	}

	// Check if interrupting.
	W_FTUEInterupter->SetVisibility(Dialogue.bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Set navigation buttons.
	Btn_Next->SetButtonText( 
		(DialogueIndex < CachedDialogues.Num() - 1) ?
		LOCTEXT("Next", "Next") :
		LOCTEXT("Close", "Close"));
	Btn_Prev->SetVisibility(
		(DialogueIndex > 0) ?
		ESlateVisibility::Visible :
		ESlateVisibility::Collapsed);

	// Set dialogue message.
	FFormatNamedArguments Arg;
	Txt_Message->SetText(FText::Format(Dialogue.Message, Arg));

	// Setup the action buttons.
	Btn_Action1->SetVisibility(ESlateVisibility::Collapsed);
	Btn_Action2->SetVisibility(ESlateVisibility::Collapsed);
	switch (Dialogue.ButtonType)
	{
	case FFTUEDialogueButtonType::TWO_BUTTONS:
		InitializeActionButton(Btn_Action2, Dialogue.Button2);
	case FFTUEDialogueButtonType::ONE_BUTTON:
		InitializeActionButton(Btn_Action1, Dialogue.Button1);
		break;
	}

	// Set dialogue position.
	if (UCanvasPanelSlot* WidgetSlot = Cast<UCanvasPanelSlot>(W_FTUEDialogue->Slot)) 
	{
		WidgetSlot->SetAutoSize(true);
		WidgetSlot->SetAnchors(FAnchors(Dialogue.GetAnchor().X, Dialogue.GetAnchor().Y));
		WidgetSlot->SetAlignment(Dialogue.GetAnchor());
		WidgetSlot->SetPosition(Dialogue.Position);
	}
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
		Button->OnClicked().AddWeakLambda(this, [ButtonModel]()
		{
			if (ButtonModel.TargetURL.IsEmpty())
			{
				// TODO: Refactor the logs.
				UE_LOG(LogTemp, Warning, TEXT("Cannot open hyperlink. Dialogue button's hyperlink is empty."));
				return;
			}

			FPlatformProcess::LaunchURL(*ButtonModel.TargetURL, nullptr, nullptr);
		});
	}
	// Bind action to execute custom function.
	else if (ButtonModel.ButtonActionType == EFTUEDialogueButtonActionType::ACTION_BUTTON)
	{
		Button->OnClicked().AddWeakLambda(this, [ButtonModel]()
		{
			if (!ButtonModel.ButtonActionDelegate.IsBound()) 
			{
				// TODO: Refactor the logs.
				UE_LOG(LogTemp, Warning, TEXT("Cannot execute custom action. Dialogue button is not bound to any event."));
				return;
			}

			ButtonModel.ButtonActionDelegate.ExecuteIfBound();
		});
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