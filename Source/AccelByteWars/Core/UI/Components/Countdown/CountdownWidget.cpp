// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/Components/Countdown/CountdownWidget.h"

#include "Core/GameStates/AccelByteWarsGameState.h"

UCountdownWidget::UCountdownWidget(const FObjectInitializer& ObjectInitializer) : UAccelByteWarsActivatableWidget(ObjectInitializer)
{
	bAutoActivate = true;
	bClosing = false;
	bHasFinished = false;
	ClosingElapsed = 0.0f;
}

void UCountdownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bHasFinished)
	{
		if (!CheckCountdownStateDelegate.IsBound())
		{
			return;
		}

		switch (CheckCountdownStateDelegate.Execute())
		{
		case ECountdownState::PRE:
			WidgetSwitcher_Root->SetActiveWidget(Text_PreCountdown);
			break;
		case ECountdownState::COUNTING:
			ChangeWidgetVisibility(true);
			WidgetSwitcher_Root->SetActiveWidget(Panel_Countdown);
			if (UpdateCountdownValueDelegate.IsBound())
			{
				int CountdownValue = UpdateCountdownValueDelegate.Execute();
				CountdownValue = CountdownValue < 0 ? 0 : CountdownValue;
				Text_CountdownValue->SetText(FText::FromString(FString::FromInt(CountdownValue)));
			}
			break;
		case ECountdownState::POST:
			bHasFinished = true;
			WidgetSwitcher_Root->SetActiveWidget(Text_PostCountdown);
			OnCountdownFinishedDelegate.Broadcast();
			CollapseWidgetWithTimer();
			break;
		case ECountdownState::INVALID:
			ChangeWidgetVisibility(false);
			break;
		default: ;
		}
	}
	else
	{
		if (bClosing)
		{
			// fade out
			ClosingElapsed += InDeltaTime;
			SetRenderOpacity(1 - (ClosingElapsed / ClosingDuration));
			if (ClosingElapsed >= ClosingDuration)
			{
				// tick wont be executed on a collapsed widget
				ChangeWidgetVisibility(false);
			}
		}
	}
}

void UCountdownWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ByteWarsGameState = Cast<AAccelByteWarsGameState>(GetWorld()->GetGameState());
}

void UCountdownWidget::SetupWidget(
	const FText PreCountdownText,
	const FText PostCountdownText,
	const FText CountdownText,
	const bool bInForceTick)
{
	bAutoActivate = true;
	bClosing = false;
	bHasFinished = false;
	ClosingElapsed = 0.0f;
	bForceTick = bInForceTick;
	ChangeWidgetVisibility(true);
	SetRenderOpacity(1);

	Text_PreCountdown->SetText(PreCountdownText);
	Text_PostCountdown->SetText(PostCountdownText);
	Text_CountdownDescription->SetText(CountdownText);

	Text_CountdownDescription->SetVisibility(
		CountdownText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UCountdownWidget::CollapseWidgetWithTimer()
{
	// start delay to collapsed this widget
	if (!GetWorld()->GetTimerManager().IsTimerActive(CollapseWidgetTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(
			CollapseWidgetTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				bClosing = true;
			}),
			1.0f,
			false,
			1.0f);
	}
}

void UCountdownWidget::ChangeWidgetVisibility(const bool bVisible)
{
	const ESlateVisibility TargetVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (bForceTick)
	{
		Widget_Outer->SetVisibility(TargetVisibility);
	}
	else
	{
		SetVisibility(TargetVisibility);
	}
}
