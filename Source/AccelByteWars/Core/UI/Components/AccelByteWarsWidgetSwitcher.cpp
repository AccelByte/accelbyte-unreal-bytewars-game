// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "CommonButtonBase.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "CommonTabListWidgetBase.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsWidgetSwitcher);

void UAccelByteWarsWidgetSwitcher::SetWidgetState(const EAccelByteWarsWidgetSwitcherState State)
{
	FText TargetText{};
	UTextBlock* TargetTextBlock = nullptr;
	bool bShowCancelButton = false, bShowRetryButton = false;

	// Determine widget to show based on state.
	UWidget* TargetWidget = GetTargetWidget(State);
	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		bShowCancelButton = bShowCancelButtonOnLoading;
		bShowRetryButton = bShowRetryButtonOnLoading;
		TargetTextBlock = Tb_Loading;
		TargetText = LoadingMessage;
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		bShowCancelButton = bShowCancelButtonOnEmpty;
		bShowRetryButton = bShowRetryButtonOnEmpty;
		TargetTextBlock = Tb_Empty;
		TargetText = EmptyMessage;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		bShowCancelButton = bShowCancelButtonOnNotEmpty;
		bShowRetryButton = bShowRetryButtonOnNotEmpty;
		break;
	case EAccelByteWarsWidgetSwitcherState::Error:
		bShowCancelButton = bShowCancelButtonOnError;
		bShowRetryButton = bShowRetryButtonOnError;
		TargetTextBlock = Tb_Error;
		TargetText = ErrorMessage;
		break;
	}

	// Mark widget state as pending if the transition animation is currently playing.
	if (Ws_Root->IsTransitionPlaying() && Ws_Root->GetActiveWidgetIndex() != GetStateWidgetIndex(State))
	{
		// Disable the animation transition to immediately show the pending states later.
		Ws_Root->SetDisableTransitionAnimation(true);

		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Log, TEXT("Widget switcher is currently transitioning. Mark new state as pending: %s"), *UEnum::GetValueAsString(State));
		PendingStates.Add(State);
		return;
	}

	// Abort if the same state is already displayed or target widget is invalid.
	const bool IsCurrentStateWidgetActive = Ws_Root->GetActiveWidgetIndex() == GetStateWidgetIndex(CurrentState);
	if ((IsCurrentStateWidgetActive && CurrentState == State) || !TargetWidget)
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Log, TEXT("Abort to switch widget switcher state. New state is already displayed: %s"), *UEnum::GetValueAsString(State));
		return;
	}

	// Display action buttons.
	Btn_Cancel->SetVisibility(bShowCancelButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Btn_Retry->SetVisibility(bShowRetryButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Btn_Cancel->SetIsEnabled(bEnableCancelButton);
	Btn_Retry->SetIsEnabled(bEnableRetryButton);

	// Only set text if different than the one already displayed
	if (TargetTextBlock != nullptr && !TargetTextBlock->GetText().EqualToCaseIgnored(TargetText))
	{
		TargetTextBlock->SetText(TargetText);
	}

	// Switch state.
	CurrentState = State;
	ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, TargetWidget, State]()
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Log, TEXT("Set widget switcher state to: %s"), *UEnum::GetValueAsString(State));
		Ws_Root->SetActiveWidget(TargetWidget);
	
		// Refresh player focus.
		if (UWidget* FocusTarget = GetFocusTargetBasedOnCurrentState(); HasUserFocus(GetOwningPlayer()) && FocusTarget)
		{
			FocusTarget->SetUserFocus(GetOwningPlayer());
		}

		// Handle widget validators on switcher state is updated.
		HandleWidgetValidators();

		// Handle FTUE on switcher state is updated.
		HandleFTUE();
	}));
}

void UAccelByteWarsWidgetSwitcher::ForceRefresh()
{
	if (CurrentState == EAccelByteWarsWidgetSwitcherState::None)
	{
		UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Warning, TEXT("Cannot force refresh widget switcher state. Widget switcher has invalid current state"));
		return;
	}

	const EAccelByteWarsWidgetSwitcherState RefreshState = CurrentState;
	CurrentState = EAccelByteWarsWidgetSwitcherState::None;

	Ws_Root->SetDisableTransitionAnimation(true);
	SetWidgetState(RefreshState);
}

void UAccelByteWarsWidgetSwitcher::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		SetWidgetState(DefaultState);
	}
}

void UAccelByteWarsWidgetSwitcher::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentState = EAccelByteWarsWidgetSwitcherState::None;
	PendingStates.Empty();

	Btn_Cancel->OnClicked().AddWeakLambda(this, [this]()
	{
		OnCancelClicked.Broadcast();
	});
	Btn_Retry->OnClicked().AddWeakLambda(this, [this]()
	{
		OnRetryClicked.Broadcast();
	});

	Ws_Root->OnActiveWidgetIndexChanged.AddUObject(this, &ThisClass::OnActiveWidgetIndexChanged);
	SetWidgetState(DefaultState);
}

void UAccelByteWarsWidgetSwitcher::NativeDestruct()
{
	CurrentState = EAccelByteWarsWidgetSwitcherState::None;
	PendingStates.Empty();

	Btn_Cancel->OnClicked().Clear();
	Btn_Retry->OnClicked().Clear();

	Ws_Root->OnActiveWidgetIndexChanged.Clear();

	Super::NativeDestruct();
}

void UAccelByteWarsWidgetSwitcher::OnActiveWidgetIndexChanged(UWidget* Widget, int32 Index)
{
	// Show next pending state.
	if (!PendingStates.IsEmpty())
	{
		const EAccelByteWarsWidgetSwitcherState NextState = PendingStates[0];
		UE_LOG(LogTemp, Warning, TEXT("Widget switcher has pending state. Switch to pending state: %s"), *UEnum::GetValueAsString(NextState));

		PendingStates.RemoveAt(0, 1, false);
		SetWidgetState(NextState);
	}
	// Force to show the current state if the active widget is incorrect due to Common UI animation race condition.
	else if (Index != GetStateWidgetIndex(CurrentState))
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget switcher has incorrect active widget. Switch to the correct current state: %s"), *UEnum::GetValueAsString(CurrentState));
		Ws_Root->SetDisableTransitionAnimation(true);
		SetWidgetState(CurrentState);
	}
	// If the state is correctly displayed as active widget, then reset the transition animation.
	else
	{
		Ws_Root->SetDisableTransitionAnimation(false);
	}
}

UWidget* UAccelByteWarsWidgetSwitcher::GetTargetWidget(const EAccelByteWarsWidgetSwitcherState State) const
{
	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		return W_Loading;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		return W_Empty;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		return Ns_NotEmpty;
	case EAccelByteWarsWidgetSwitcherState::Error:
		return W_Error;
	}

	return nullptr;
}

int32 UAccelByteWarsWidgetSwitcher::GetStateWidgetIndex(const EAccelByteWarsWidgetSwitcherState State) const
{
	return Ws_Root->GetChildIndex(GetTargetWidget(State));
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

void UAccelByteWarsWidgetSwitcher::HandleWidgetValidators()
{
	UAccelByteWarsActivatableWidget* ActiveWidget = Cast<UAccelByteWarsActivatableWidget>(UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this));
	if (!ActiveWidget)
	{
		return;
	}

	ActiveWidget->ExecuteWidgetValidators();
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

	if (CurrentState == EAccelByteWarsWidgetSwitcherState::Not_Empty ||
		CurrentState == EAccelByteWarsWidgetSwitcherState::Empty)
	{
		FTUEWidget->ShowDialogues(true);
	}
	else
	{
		FTUEWidget->CloseDialogues();
	}
}

void UAccelByteWarsWidgetSwitcher::ExecuteNextTick(const FTimerDelegate& Delegate) const
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
}