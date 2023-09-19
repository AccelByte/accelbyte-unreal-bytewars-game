


#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "CommonButtonBase.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsWidgetSwitcher);

void UAccelByteWarsWidgetSwitcher::SetWidgetState(const EAccelByteWarsWidgetSwitcherState State, const bool bForce)
{
	UWidget* TargetWidget = nullptr;
	UTextBlock* TargetTextBlock = nullptr;
	FText TargetText;
	bool bShowCancelButton = false;
	bool bShowRetryButton = false;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		TargetWidget = W_Loading;
		bShowCancelButton = bShowCancelButtonOnLoading;
		bShowRetryButton = bShowRetryButtonOnLoading;
		TargetTextBlock = Tb_Loading;
		TargetText = LoadingMessage;
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		TargetWidget = W_Empty;
		bShowCancelButton = bShowCancelButtonOnEmpty;
		bShowRetryButton = bShowRetryButtonOnEmpty;
		TargetTextBlock = Tb_Empty;
		TargetText = EmptyMessage;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		TargetWidget = Ns_NotEmpty;
		bShowCancelButton = bShowCancelButtonOnNotEmpty;
		bShowRetryButton = bShowRetryButtonOnNotEmpty;
		break;
	case EAccelByteWarsWidgetSwitcherState::Error:
		TargetWidget = W_Error;
		bShowCancelButton = bShowCancelButtonOnError;
		bShowRetryButton = bShowRetryButtonOnError;
		TargetTextBlock = Tb_Error;
		TargetText = ErrorMessage;
		break;
	}

	Btn_Cancel->SetVisibility(bShowCancelButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Btn_Retry->SetVisibility(bShowRetryButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Cancel->SetIsEnabled(bEnableCancelButton);
	Btn_Retry->SetIsEnabled(bEnableRetryButton);

	/**
	 * CurrentState is used as a workaround for GetActiveWidet.
	 * Since the active widget will actually be set on the end of the tick.
	 */
	if (TargetWidget != nullptr && (CurrentState != State || bForce))
	{
		CurrentState = State;
		Ws_Root->SetActiveWidget(TargetWidget);
	}

	// only set text if different than the one already displayed
	if (TargetTextBlock != nullptr && (!TargetTextBlock->GetText().EqualToCaseIgnored(TargetText) || bForce))
	{
		TargetTextBlock->SetText(TargetText);
	}

	if (UWidget* FocusTarget = GetFocusTargetBasedOnCurrentState(); HasUserFocus(GetOwningPlayer()) && FocusTarget)
	{
		FocusTarget->SetUserFocus(GetOwningPlayer());
	}

	// Handle FTUE on switcher updated.
	HandleFTUE();
}

void UAccelByteWarsWidgetSwitcher::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	SetWidgetState(DefaultState, true);
}

void UAccelByteWarsWidgetSwitcher::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Cancel->OnClicked().AddWeakLambda(this, [this]()
	{
		OnCancelClicked.Broadcast();
	});
	Btn_Retry->OnClicked().AddWeakLambda(this, [this]()
	{
		OnRetryClicked.Broadcast();
	});
}

void UAccelByteWarsWidgetSwitcher::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_Cancel->OnClicked().Clear();
	Btn_Retry->OnClicked().Clear();
}

UWidget* UAccelByteWarsWidgetSwitcher::GetFocusTargetBasedOnCurrentState() const
{
	UWidget* FocusTarget = nullptr;

	switch (CurrentState)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		if (bShowCancelButtonOnLoading)
		{
			FocusTarget = Btn_Cancel;
		}
		else if (bShowRetryButtonOnLoading)
		{
			FocusTarget = Btn_Retry;
		}
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		if (bShowCancelButtonOnEmpty)
		{
			FocusTarget = Btn_Cancel;
		}
		else if (bShowCancelButtonOnEmpty)
		{
			FocusTarget = Btn_Retry;
		}
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		if (bShowCancelButtonOnNotEmpty)
		{
			FocusTarget = Btn_Cancel;
		}
		else if (bShowCancelButtonOnNotEmpty)
		{
			FocusTarget = Btn_Retry;
		}
		break;
	case EAccelByteWarsWidgetSwitcherState::Error:
		if (bShowCancelButtonOnError)
		{
			FocusTarget = Btn_Cancel;
		}
		else if (bShowCancelButtonOnError)
		{
			FocusTarget = Btn_Retry;
		}
		break;
	}

	return FocusTarget;
}

void UAccelByteWarsWidgetSwitcher::HandleFTUE()
{
	if (!bOnLoadedInitializeFTUE) 
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Warning, TEXT("Cannot handle FTUE dialogues. Game Instance is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget)
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Warning, TEXT("Cannot handle FTUE dialogues. Base UI widget is not valid."));
		return;
	}

	UFTUEDialogueWidget* FTUEWidget = BaseUIWidget->GetFTUEDialogueWidget();
	if (!FTUEWidget)
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Warning, TEXT("Cannot handle FTUE dialogues. FTUE dialogue widget is not valid."));
		return;
	}

	if (CurrentState == EAccelByteWarsWidgetSwitcherState::Not_Empty)
	{
		FTUEWidget->ShowDialoguesFirstTime();
	}
	else
	{
		FTUEWidget->CloseDialogues();
		FTUEWidget->TryToggleHelpDev(false);
	}
}
