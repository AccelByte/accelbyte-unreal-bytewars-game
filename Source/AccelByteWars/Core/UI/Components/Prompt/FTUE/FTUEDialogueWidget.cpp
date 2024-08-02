// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBoxSlot.h"

DEFINE_LOG_CATEGORY(LogFTUEDialogueWidget);

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFTUEDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());

	BaseUIWidget = GameInstance->GetBaseUIWidget(false);

	Btn_Next->OnClicked().AddUObject(this, &ThisClass::NextDialogue);
	Btn_Prev->OnClicked().AddUObject(this, &ThisClass::PrevDialogue);
	Btn_Close->OnClicked().AddUObject(this, &ThisClass::CloseDialoguesByGroup);

	Cast<UBorder>(W_FTUEInterrupter)->OnMouseButtonUpEvent.BindUFunction(this, FName("OnInterrupterClicked"));

	ResetDialogues();
}

void UFTUEDialogueWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_Next->OnClicked().Clear();
	Btn_Prev->OnClicked().Clear();
	Btn_Close->OnClicked().Clear();

	BaseUIWidget->SetFTUEDialogueWidget(nullptr);
	CloseDialogues();
}

void UFTUEDialogueWidget::AddDialogues(const TArray<FFTUEDialogueModel*>& Dialogues)
{
	// Cache dialogues as origin.
	for (FFTUEDialogueModel* Dialogue : Dialogues)
	{
		DialoguesOrigin.AddUnique(Dialogue);
	}
	
	// When the dialogue list is changed, reset the validation.
	bIsAllDialogueValidated = false;
}

bool UFTUEDialogueWidget::RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass)
{
	const int32 Removed = DialoguesOrigin.RemoveAll([WidgetClass](const FFTUEDialogueModel* Temp)
	{
		return !Temp || Temp->TargetWidgetClasses.Contains(WidgetClass);
	});
	
	// When the dialogue list is changed, reset the validation.
	bIsAllDialogueValidated = false;

	return Removed > 0;
}

void UFTUEDialogueWidget::ShowDialogues(const bool bFirstTime)
{
	if (W_FTUEDialogue->IsVisible())
	{
		return;
	}

	bIsFirstTimeMode = bFirstTime;

	// Validate dialogues and show it after the validation complete.
	if (!bIsAllDialogueValidated) 
	{
		ValidateDialogues();	
	}
	else 
	{
		OnValidateDialoguesComplete();
	}
}

void UFTUEDialogueWidget::CloseDialogues()
{
	if (!IsActivated())
	{
		return;
	}

	// Handle if close button is being spammed repeatedly.
	if (!BaseUIWidget->GetFTUEDialogueWidget())
	{
		BaseUIWidget->SetFTUEDialogueWidget(this);
	}

	if (!FMath::IsNearlyEqual(W_DarkBorder->GetRenderOpacity(), 0.0f, 0.02f))
	{
		PlayFadeOutAnimation(W_DarkBorder);
	}

	PlayFadeOutAnimation(W_FTUEDialogue, 1, [this]
	{
		ResetDialogues();
		DeactivateWidget();
		HideFTUEDevHelpInputAction(false);
	});
}

void UFTUEDialogueWidget::ResetDialogues()
{
	// Tear down FTUE.
	ClearHighlightedWidget();
	DeinitializeLastDialogue();
	DialoguesOrigin.Empty();
	DialoguesInternal.Empty();
	DialoguesByGroup.Reset();
	DialogueIndex = INDEX_NONE;
	CachedLastDialogue = nullptr;

	W_FTUEInterrupter->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_DarkBorder->SetVisibility(ESlateVisibility::Collapsed);
	W_DarkBorder->SetRenderOpacity(1.0f);
}

void UFTUEDialogueWidget::PauseDialogues() const
{
	// Simple hide FTUE to pause dialogues.
	W_FTUEDialogue->SetVisibility(ESlateVisibility::Collapsed);
	W_FTUEInterrupter->SetVisibility(ESlateVisibility::Collapsed);
	W_DarkBorder->SetVisibility(ESlateVisibility::Collapsed);
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
		W_FTUEInterrupter->SetVisibility(
			CachedLastDialogue->bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		
		W_DarkBorder->SetVisibility(
			CachedLastDialogue->bHighlightWidget ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
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
		CloseDialogues();
		return;
	}

	const TFunction<void()> InitPrevDialogue = [this]
	{
		if (!InitializeDialogue(DialoguesInternal[--DialogueIndex]))
		{
			PrevDialogue();
		}
	};

	const FFTUEDialogueModel* CurrentDialogue = DialoguesInternal[DialogueIndex];
	const FFTUEDialogueModel* TargetDialogue = DialoguesInternal[DialogueIndex - 1];

	HandleDarkBorderTransition(CurrentDialogue, TargetDialogue);

	const bool bIsPositionEqual = IsDialoguePositionEqual(CurrentDialogue, TargetDialogue);
	if (!bIsPositionEqual)
	{
		PlayFadeOutAnimation(W_FTUEDialogue, 1, [=]
		{
			InitPrevDialogue();
			PlayFadeInAnimation(W_FTUEDialogue);
		});

		return;
	}
	
	InitPrevDialogue();
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

	const TFunction<void()> InitNextDialogue = [this]
	{
		if (!InitializeDialogue(DialoguesInternal[++DialogueIndex]))
		{
			NextDialogue();
		}
	};
	
	const FFTUEDialogueModel* CurrentDialogue = DialoguesInternal[DialogueIndex];
	const FFTUEDialogueModel* TargetDialogue = DialoguesInternal[DialogueIndex + 1];

	HandleDarkBorderTransition(CurrentDialogue, TargetDialogue);

	const bool bIsDifferentGroup = CurrentDialogue->Group != TargetDialogue->Group;
	const bool bIsPositionEqual = IsDialoguePositionEqual(CurrentDialogue, TargetDialogue);

	if (!bIsPositionEqual || bIsDifferentGroup)
	{
		PlayFadeOutAnimation(W_FTUEDialogue, 1, [=]
		{
			InitNextDialogue();
			PlayFadeInAnimation(W_FTUEDialogue);
		});

		return;
	}
	
	InitNextDialogue();
}

void UFTUEDialogueWidget::JumpToDialogue(const uint_fast8_t TargetIndex)
{
	if (!W_FTUEDialogue->IsVisible()) 
	{
		return;
	}

	if (TargetIndex >= DialoguesInternal.Num())
	{
		return;
	}

	const uint_fast8_t CurrentIndex = DialogueIndex;
	DialogueIndex = TargetIndex;

	const TFunction<void()> InitTargetDialogue = [=]
	{
		if (!InitializeDialogue(DialoguesInternal[TargetIndex]))
		{
			DialogueIndex = CurrentIndex;
		}
	};

	const FFTUEDialogueModel* CurrentDialogue = DialoguesInternal[CurrentIndex];
	const FFTUEDialogueModel* TargetDialogue = DialoguesInternal[TargetIndex];

	HandleDarkBorderTransition(CurrentDialogue, TargetDialogue);

	const bool bIsPositionEqual = IsDialoguePositionEqual(CurrentDialogue, TargetDialogue);
	if (!bIsPositionEqual)
	{
		PlayFadeOutAnimation(W_FTUEDialogue, 1, [=]
		{
			InitTargetDialogue();
			PlayFadeInAnimation(W_FTUEDialogue);
		});

		return;
	}

	InitTargetDialogue();
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
		const IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(Dialogue->HighlightedWidget);
		if (WidgetInterface == nullptr)
		{
			UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Highlighted widget is found but it is not implementing widget highlighting interface."));
			return false;
		}
		
		// Highlight widget.
		CachedHighlightedWidget = Dialogue->HighlightedWidget;
		WidgetInterface->Execute_ToggleHighlight(Dialogue->HighlightedWidget, true);
	}

	// Check if interrupting.
	W_FTUEInterrupter->SetVisibility(Dialogue->bIsInterrupting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	W_DarkBorder->SetVisibility(Dialogue->bIsInterrupting ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	// Set navigation buttons.
	InitializeNextButton(Dialogue);
	InitializePrevButton(Dialogue);
	InitializeCloseButton();

	InitializePageNumber();

	// Set dialogue message.
	Tb_Message->SetText(Dialogue->GetFormattedMessage());

	// Set up the action buttons.
	Btn_Action1->SetVisibility(ESlateVisibility::Collapsed);
	Btn_Action2->SetVisibility(ESlateVisibility::Collapsed);
	switch (Dialogue->ButtonType)
	{
	case FFTUEDialogueButtonType::TWO_BUTTONS:
		Cast<UHorizontalBoxSlot>(Btn_Action1->Slot)->SetPadding(FMargin());
		InitializeActionButton(Btn_Action2, Dialogue->Button2);
		InitializeActionButton(Btn_Action1, Dialogue->Button1);
		break;
	case FFTUEDialogueButtonType::ONE_BUTTON:
		Cast<UHorizontalBoxSlot>(Btn_Action1->Slot)->SetPadding(FMargin(200.0f, 0.0f));
		InitializeActionButton(Btn_Action1, Dialogue->Button1);
		break;
	case FFTUEDialogueButtonType::NO_BUTTON:
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

void UFTUEDialogueWidget::InitializeNextButton(const FFTUEDialogueModel* Dialogue) const
{
	const FFTUEDialogueModel* LastDialogueInGroup = DialoguesByGroup.Find(Dialogue->Group)->Last();
	const bool bIsDialogueLastInGroup = Dialogue == LastDialogueInGroup;

	Btn_Next->SetButtonText(bIsDialogueLastInGroup ? LOCTEXT("Done", "Done") : LOCTEXT("Next", "Next"));
	if (!Btn_Next->IsVisible())
	{
		Btn_Next->SetVisibility(ESlateVisibility::Visible);
	}

	Btn_Next->SetFocus();
}

void UFTUEDialogueWidget::InitializePrevButton(const FFTUEDialogueModel* Dialogue) const
{
	const FFTUEDialogueModel* FirstDialogueInGroup = (*DialoguesByGroup.Find(Dialogue->Group))[0];
	const bool bIsDialogueFirstInGroup = Dialogue == FirstDialogueInGroup;

	Btn_Prev->SetButtonText(LOCTEXT("Prev", "Prev"));
	Btn_Prev->SetVisibility(bIsDialogueFirstInGroup ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UFTUEDialogueWidget::InitializeCloseButton() const
{
	Btn_Close->SetVisibility(ESlateVisibility::Visible);
	Btn_Close->SetTriggeringInputAction(CloseButtonInputActionData);
	Btn_Close->SetIsFocusable(false);
}

void UFTUEDialogueWidget::InitializePageNumber() const
{
    const TArray<FFTUEDialogueModel*>* DialoguesInGroup = DialoguesByGroup.Find(DialoguesInternal[DialogueIndex]->Group);

    if (DialoguesInGroup && DialoguesInGroup->Num() > 1)
    {
	    const int32 IndexInGroup = DialoguesInGroup->IndexOfByPredicate([&](const FFTUEDialogueModel* Dialogue)
        {
            return Dialogue == DialoguesInternal[DialogueIndex];
        });

        if (IndexInGroup != INDEX_NONE)
        {
            const FText PageNumberText = FText::Format(
                LOCTEXT("PageNumber", "{0} of {1}"), IndexInGroup + 1, DialoguesInGroup->Num());

            Tb_PageNumber->SetText(PageNumberText);
            return;
        }
    }

    Tb_PageNumber->SetText(FText());
}

void UFTUEDialogueWidget::ValidateDialogues()
{
	// Clear cached dialogues.
	DialoguesInternal.Empty();

	DialoguesOrigin.RemoveAll([this](FFTUEDialogueModel* Dialogue)
	{
		if (!Dialogue)
		{
			return true;
		}
		
		// Remove invalid dialogues from the original input.
		if (!Dialogue->OwnerTutorialModule || !Dialogue->OwnerTutorialModule->IsActiveAndDependenciesChecked())
		{
			return true;
		}

		if (!Dialogue->bHighlightWidget)
	 	{
	 		return false;
	 	}
	
		// Remove invalid dialogues if the widget to highlight is not found.
	 	const FString WidgetToHighlightString = Dialogue->TargetWidgetNameToHighlight;
	 	if (WidgetToHighlightString.IsEmpty())
	 	{
	 		UE_LOG_FTUEDIALOGUEWIDGET(Warning, TEXT("Cannot validate dialogue. Widget to highlight is empty."));
	 		return true;
	 	}
	
	 	TArray<UUserWidget*> FoundWidgets = 
			AccelByteWarsUtility::FindWidgetsOnTheScreen(
				WidgetToHighlightString, 
				Dialogue->TargetWidgetClassToHighlight.Get(), 
				false, 
				this);

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
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

			const bool bIsWidgetFound = FoundWidget->GetName().Equals(WidgetToHighlightString);
			if (!bIsWidgetFound)
			{
				continue;
			}

			UE_LOG_FTUEDIALOGUEWIDGET(Log, TEXT("Highlighted widget found."));
			Dialogue->HighlightedWidget = FoundWidget;
			return false;
		}

		return FoundWidgets.IsEmpty();
	});

	// Abort if no valid dialogues from the original input.
	if (DialoguesOrigin.IsEmpty()) 
	{
		CloseDialogues();
		return;
	}

	// Execute validation for each dialogues recursively.
	ValidateDialogue(0);
}

void UFTUEDialogueWidget::OnValidateDialoguesComplete()
{
	bIsAllDialogueValidated = true;

	// Check whether to abort if all dialogues are already shown.
	if (bIsFirstTimeMode)
	{
		// Reset is already shown dialogue statuses if always show FTUE.
		if (GameInstance && GameInstance->GetFTUEAlwaysOnSetting())
		{
			TArray<FFTUEDialogueModel*> Instigators = DialoguesInternal.FilterByPredicate([](const FFTUEDialogueModel* Temp)
			{
				return Temp && Temp->bIsInstigator;
			});

			for (const FFTUEDialogueModel* Instigator : Instigators)
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
			CloseDialogues();
			return;
		}
	}

	// Abort if after validation, the dialogues turns out to be empty.
	if (DialoguesInternal.IsEmpty())
	{
		CloseDialogues();
		return;
	}

	// Reinitialize FTUE helpers.
	ClearHighlightedWidget();
	CachedLastDialogue = nullptr;
	
	/* If first time mode, get the first dialogue that has not yet shown.
	 * If not, fallback to the first dialogue.*/
	DialogueIndex = 0;
	if (bIsFirstTimeMode) 
	{
		for (int32 i = 0; i < DialoguesInternal.Num(); i++)
		{
			if (!DialoguesInternal[i]->bIsAlreadyShown)
			{
				DialogueIndex = i;
				break;
			}
		}
	}

	// Show FTUE.
	if (InitializeDialogue(DialoguesInternal[DialogueIndex]))
	{
		if (DialoguesInternal[DialogueIndex]->bHighlightWidget)
		{
			W_DarkBorder->SetRenderOpacity(0.0f);
		}
		else
		{
			PlayFadeInAnimation(W_DarkBorder);
		}
		W_FTUEDialogue->SetVisibility(ESlateVisibility::Visible);
		PlayFadeInAnimation(W_FTUEDialogue);
	}
	else
	{
		CloseDialogues();
	}
}

void UFTUEDialogueWidget::ValidateDialogue(const int32 DialogueToValidateIndex)
{
	// Finalize validation if reached invalid index or the end of the dialogue array.
	const bool bIsValidIndex = DialoguesOrigin.IsValidIndex(DialogueToValidateIndex);
	if (!bIsValidIndex)
	{
		OnValidateDialoguesComplete();
		return;
	}

	const int32 NextDialogueIndex = DialogueToValidateIndex + 1;

	// Skip to next dialogue if the current one is invalid.
	FFTUEDialogueModel* Dialogue = DialoguesOrigin[DialogueToValidateIndex];
	if (!Dialogue) 
	{
		ValidateDialogue(NextDialogueIndex);
		return;
	}

	// Execute dialogue validation.
	Dialogue->ExecuteValidation(FOnFTUEDialogueValidationComplete::CreateUObject(this, &ThisClass::OnValidateDialogueComplete, NextDialogueIndex), this);
}

void UFTUEDialogueWidget::OnValidateDialogueComplete(FFTUEDialogueModel* Dialogue, const bool bIsValid, const int32 NextDialogueToValidateIndex)
{
	// Add dialogue to internal cache if valid.
	if (bIsValid && Dialogue)
	{
		uint_fast8_t SortIndex = 0;
		for (; SortIndex < DialoguesInternal.Num(); SortIndex++) 
		{
			const FFTUEDialogueModel* CompareDialogue = DialoguesInternal[SortIndex];
			if (Dialogue->GroupOrderPriority > CompareDialogue->GroupOrderPriority) 
			{
				break;
			}
		}
		DialoguesInternal.Insert(Dialogue, SortIndex);
		
		TArray<FFTUEDialogueModel*>& DialoguesInGroup = DialoguesByGroup.FindOrAdd(Dialogue->Group);
		DialoguesInGroup.Add(Dialogue);
	}
	// Execute validation for next dialogue.
	ValidateDialogue(NextDialogueToValidateIndex);
}

void UFTUEDialogueWidget::DeinitializeLastDialogue() const
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
		const IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(CachedHighlightedWidget);
		if (!WidgetInterface)
		{
			return;
		}
		WidgetInterface->Execute_ToggleHighlight(CachedHighlightedWidget, false);
	}

	CachedHighlightedWidget = nullptr;
}

void UFTUEDialogueWidget::CloseDialoguesByGroup()
{
    const FFTUEDialogueGroup* CurrentGroup = DialoguesInternal[DialogueIndex]->Group;
    uint_fast8_t NextGroupIndex = DialoguesInternal.Num();

    for (uint_fast8_t Index = DialogueIndex + 1; Index < DialoguesInternal.Num(); Index++)
    {
        if (DialoguesInternal[Index]->Group != CurrentGroup)
        {
            NextGroupIndex = Index;
            break;
        }
    }

    if (NextGroupIndex == DialoguesInternal.Num())
    {
        CloseDialogues();
        return;
    }

    PlayFadeOutAnimation(W_FTUEDialogue, 1, [this, NextGroupIndex]
    {
        JumpToDialogue(NextGroupIndex);
        PlayFadeInAnimation(W_FTUEDialogue);
    });
}

void UFTUEDialogueWidget::HandleDarkBorderTransition(const FFTUEDialogueModel* CurrentDialogue,
	const FFTUEDialogueModel* TargetDialogue, const TFunction<void()>& OnComplete) const
{
	const bool bHasHighlight = CurrentDialogue->bHighlightWidget;
	const bool bTargetHasHighlight = TargetDialogue->bHighlightWidget;

	if (!bHasHighlight && bTargetHasHighlight)
	{
		PlayFadeOutAnimation(W_DarkBorder, 1, OnComplete);
		return;
	}

	if (bHasHighlight && !bTargetHasHighlight)
	{
		PlayFadeInAnimation(W_DarkBorder, 1, OnComplete);
		return;
	}

	if (OnComplete)
	{
		OnComplete();
	}
}

void UFTUEDialogueWidget::OnInterrupterClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	if (!W_FTUEDialogue->IsVisible())
	{
		return;
	}

	PlayZoomInOutAnimation(W_FTUEDialogue, 1.0f, {}, 1);
}

bool UFTUEDialogueWidget::IsAllDialoguesAlreadyShown() const
{
	const TArray<FFTUEDialogueModel*> AlreadyShown = DialoguesInternal.FilterByPredicate([](const FFTUEDialogueModel* Temp)
	{
		return Temp && Temp->bIsAlreadyShown;
	});

	return AlreadyShown.Num() == DialoguesInternal.Num();
}

bool UFTUEDialogueWidget::IsDialoguePositionEqual(const FFTUEDialogueModel* Dialogue,
	const FFTUEDialogueModel* TargetDialogue)
{
	return Dialogue->Position == TargetDialogue->Position;
}

void UFTUEDialogueWidget::PlayFadeInAnimation(UWidget* TargetWidget, const float AnimationSpeedModifier, 
	const TFunction<void()>& OnAnimationCompleted, const bool bEnableShrinkIn)
{
	if (!IsValid(TargetWidget))
    {
        return;
    }

	const float AnimationDelay = 0.001f / AnimationSpeedModifier;
	constexpr float ScaleModifier = 0.0005f;

	constexpr float InitialOpacity = 0.0f;
	constexpr float TargetOpacity = 1.0f;
	constexpr float OpacityModifier = 0.01f;
	constexpr float ExpandedScale = ScaleModifier * (TargetOpacity / OpacityModifier);

	TargetWidget->SetIsEnabled(true);

	Async(EAsyncExecution::TaskGraph, [=]
	{
		FWidgetTransform InRenderTransform = FWidgetTransform();
		if (bEnableShrinkIn)
		{
			InRenderTransform.Scale += FVector2D(ExpandedScale, ExpandedScale);
		}

		for (float CurrentOpacity = InitialOpacity; CurrentOpacity <= TargetOpacity; CurrentOpacity += OpacityModifier)
		{
			if (bEnableShrinkIn)
			{
				InRenderTransform.Scale -= FVector2D(ScaleModifier, ScaleModifier);
			}

			SetWidgetOpacityAndTransform(TargetWidget, CurrentOpacity, InRenderTransform);
			FPlatformProcess::Sleep(AnimationDelay);
		}

		if (!IsValid(TargetWidget) || !OnAnimationCompleted)
		{
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [=]
		{
			OnAnimationCompleted();
		});
	});
}

void UFTUEDialogueWidget::PlayFadeOutAnimation(UWidget* TargetWidget, const float AnimationSpeedModifier, 
	const TFunction<void()>& OnAnimationCompleted, const bool bEnableExpandOut)
{
	if (!IsValid(TargetWidget))
    {
        return;
    }

	const float AnimationDelay = 0.001f / AnimationSpeedModifier;
	constexpr float ScaleModifier = 0.0005f;

	constexpr float InitialOpacity = 1.0f;
	constexpr float TargetOpacity = 0.0f;
	constexpr float OpacityModifier = 0.01f;

	TargetWidget->SetIsEnabled(false);

	Async(EAsyncExecution::TaskGraph, [=]
	{
        FWidgetTransform InRenderTransform = FWidgetTransform();
		for (float CurrentOpacity = InitialOpacity; CurrentOpacity >= TargetOpacity; CurrentOpacity -= OpacityModifier)
		{
			if (bEnableExpandOut)
			{
				InRenderTransform.Scale += FVector2D(ScaleModifier, ScaleModifier);
			}

			SetWidgetOpacityAndTransform(TargetWidget, CurrentOpacity, InRenderTransform);
			FPlatformProcess::Sleep(AnimationDelay);
		}
		
		if (!IsValid(TargetWidget) || !OnAnimationCompleted)
		{
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [=]
		{
			OnAnimationCompleted();
		});
	});
}

void UFTUEDialogueWidget::PlayZoomInOutAnimation(UWidget* TargetWidget, const float AnimationSpeedModifier,
	const TFunction<void()>& OnAnimationCompleted, const int BounceCount)
{
	if (!IsValid(TargetWidget))
	{
		return;
	}

	const float AnimationDelay = 0.001f / AnimationSpeedModifier;
	constexpr float ScaleModifier = 0.0005f;
	constexpr float MaxScale = 1.025f;

	Async(EAsyncExecution::TaskGraph, [=]
	{
		FWidgetTransform InRenderTransform = FWidgetTransform();
		for (int BounceIndex = 0; BounceIndex < BounceCount; BounceIndex++)
		{
			for (float CurrentScale = 1.0f; CurrentScale <= MaxScale; CurrentScale += ScaleModifier)
			{
				InRenderTransform.Scale = FVector2D(CurrentScale, CurrentScale);
				SetWidgetOpacityAndTransform(TargetWidget, 1.0f, InRenderTransform);
				FPlatformProcess::Sleep(AnimationDelay);
			}

			for (float CurrentScale = MaxScale; CurrentScale >= 1.0f; CurrentScale -= ScaleModifier)
			{
				InRenderTransform.Scale = FVector2D(CurrentScale, CurrentScale);
				SetWidgetOpacityAndTransform(TargetWidget, 1.0f, InRenderTransform);
				FPlatformProcess::Sleep(AnimationDelay);
			}
		}

		if (!IsValid(TargetWidget) || !OnAnimationCompleted)
		{
			return;
		}

		AsyncTask(ENamedThreads::GameThread, [=]
		{
			OnAnimationCompleted();
		});
	});
}

void UFTUEDialogueWidget::SetWidgetOpacityAndTransform(UWidget* TargetWidget, const float Opacity,
	const FWidgetTransform& Transform)
{
	AsyncTask(ENamedThreads::GameThread, [=]
	{
		if (!IsValid(TargetWidget))
		{
			return;
		}

		TargetWidget->SetRenderTransform(Transform);
		TargetWidget->SetRenderOpacity(Opacity);
	});
}

bool UFTUEDialogueWidget::NativeOnHandleBackAction()
{
	if (!W_FTUEDialogue->IsVisible())
	{
		return false;
	}

	NextDialogue();
	return true;
}

#undef LOCTEXT_NAMESPACE