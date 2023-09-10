// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/BubbleDialogueWidget.h"

#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/TextBlock.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UBubbleDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Open->OnClicked().Clear();
	Btn_Open->OnClicked().AddUObject(this, &ThisClass::ShowDialogues);

	Btn_Close->OnClicked().Clear();
	Btn_Close->OnClicked().AddUObject(this, &ThisClass::CloseDialogues);

	// On initialize, close the FTUE.
	W_FTUE->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
}

void UBubbleDialogueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void UBubbleDialogueWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

void UBubbleDialogueWidget::AddDialogues(TArray<FFTUEDialogueModel> Dialogues)
{
	CachedDialogues.Append(Dialogues);
}

void UBubbleDialogueWidget::RemoveAssociateDialogues(const UTutorialModuleDataAsset* TutorialModule)
{
	CachedDialogues.RemoveAll([TutorialModule](const FFTUEDialogueModel Temp)
	{
		return Temp.OwnerTutorialModule == TutorialModule;
	});

	CloseDialogues();
}

void UBubbleDialogueWidget::ShowDialogues()
{
	DialogueIndex = 0;

	if (CachedDialogues.IsEmpty()) 
	{
		return;
	}

	CachedDialogues.Sort();
	InitializeDialogue(CachedDialogues[DialogueIndex]);
	W_FTUE->SetVisibility(ESlateVisibility::Visible);
}

void UBubbleDialogueWidget::CloseDialogues()
{
	// Clear last highlighted widget if any.
	if (CachedHighlightedWidget)
	{
		IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(CachedHighlightedWidget);
		if (WidgetInterface)
		{
			WidgetInterface->Execute_ToggleHighlight(CachedHighlightedWidget, false);
		}
	}

	// If no dialogue left, close the FTUE.
	if (DialogueIndex >= CachedDialogues.Num() - 1) 
	{
		W_FTUE->SetVisibility(ESlateVisibility::Collapsed);
		W_FTUEInterupter->SetVisibility(ESlateVisibility::Collapsed);
	}
	// Else, open the next dialogue.
	else 
	{
		NextDialogue();
	}
}

void UBubbleDialogueWidget::NextDialogue()
{
	if (DialogueIndex >= CachedDialogues.Num() - 1) 
	{
		DialogueIndex = CachedDialogues.Num() - 1;
		return;
	}

	InitializeDialogue(CachedDialogues[++DialogueIndex]);
}

void UBubbleDialogueWidget::PrevDialogue()
{
	if (DialogueIndex <= 0)
	{
		DialogueIndex = 0;
		return;
	}

	InitializeDialogue(CachedDialogues[--DialogueIndex]);
}

void UBubbleDialogueWidget::InitializeDialogue(const FFTUEDialogueModel& Dialogue)
{
	// Set dialogue message.
	Txt_Message->SetText(Dialogue.Message);

	// Setup the action buttons.
	switch (Dialogue.ButtonType)
	{
	case FFTUEDialogueButtonType::TWO_BUTTONS:
		InitializeActionButton(Btn_Action2, Dialogue.Button2);
	case FFTUEDialogueButtonType::ONE_BUTTON:
		InitializeActionButton(Btn_Action1, Dialogue.Button1);
		break;
	default:
		Btn_Action1->SetVisibility(ESlateVisibility::Collapsed);
		Btn_Action2->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}

	// Check for highlighted widget.
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
	}

	// Check if interrupting.
	W_FTUEInterupter->SetVisibility(Dialogue.bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Set dialogue position.
	UCanvasPanelSlot* Widget = Canvas_FTUE->AddChildToCanvas(W_FTUE);
	Widget->SetAutoSize(true);
	Widget->SetAnchors(FAnchors(Dialogue.GetAnchor().X, Dialogue.GetAnchor().Y));
	Widget->SetAlignment(Dialogue.GetAnchor());
	Widget->SetPosition(Dialogue.Position);
}

void UBubbleDialogueWidget::InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel)
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
