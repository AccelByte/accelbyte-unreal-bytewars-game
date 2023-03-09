// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/UI/InGameMenu/HUD/HUDWidget.h"

#include "HUDWidgetEntry.h"
#include "Components/TextBlock.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ByteWarsGameState = GetWorld()->GetGameState<AAccelByteWarsGameStateBase>();

	// setup pre game countdown
	Widget_PreGameCountdown->SetupWidget(
		FText::FromString("Waiting for all players"),
		FText::FromString("Starting Game"));
	Widget_PreGameCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetPreGameCountdownState);
	Widget_PreGameCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdatePreGameCountdownValue);
	OnPreGameCountdownFinishedDelegateHandle =
		Widget_PreGameCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnPreGameCountdownFinished);

	// setup not enough player countdown
	Widget_NotEnoughPlayerCountdown->SetupWidget(
		FText::FromString(""),
		FText::FromString(""),
		FText::FromString("Not enough players | Shutting down in: "),
		true);
	Widget_NotEnoughPlayerCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetNotEnoughPlayerCountdownState);
	Widget_NotEnoughPlayerCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdateNotEnoughPlayerCountdownValue);
	OnNotEnoughPlayerCountdownFinishedDelegateHandle =
		Widget_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnNotEnoughPlayerCountdownFinished);
}

void UHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Widget_PreGameCountdown->OnCountdownFinishedDelegate.Remove(OnPreGameCountdownFinishedDelegateHandle);
	Widget_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.Remove(OnNotEnoughPlayerCountdownFinishedDelegateHandle);
}

void UHUDWidget::SetValue(const FString Value, const int32 Index, const int32 BoxIndex)
{
	if (Index < 0 || Index > 3)
	{
		return;
	}

	if (BoxIndex < 0 || BoxIndex > 2)
	{
		return;
	}

	const UHUDWidgetEntry* TargetWidget;
	switch (Index)
	{
	case 0:
		TargetWidget = Widget_HUDNameValueP1;
		break;
	case 1:
		TargetWidget = Widget_HUDNameValueP2;
		break;
	case 2:
		TargetWidget = Widget_HUDNameValueP3;
		break;
	case 3:
		TargetWidget = Widget_HUDNameValueP4;
		break;
	default:
		return;
	}

	UTextBlock* TargetTextBlock;
	switch (BoxIndex)
	{
	case 0:
		TargetTextBlock = TargetWidget->Text_Value_Left;
		break;
	case 1:
		TargetTextBlock = TargetWidget->Text_Value_Middle;
		break;
	case 2:
		TargetTextBlock = TargetWidget->Text_Value_Right;
		break;
	default:
		return;
	}

	TargetTextBlock->SetText(FText::FromString(Value));
}

void UHUDWidget::SetColorChecked(const int32 Index, const FLinearColor Color)
{
	if (Index < 0 || Index > 3)
	{
		return;
	}

	UHUDWidgetEntry* TargetWidget;
	switch (Index)
	{
	case 0:
		TargetWidget = Widget_HUDNameValueP1;
		break;
	case 1:
		TargetWidget = Widget_HUDNameValueP2;
		break;
	case 2:
		TargetWidget = Widget_HUDNameValueP3;
		break;
	case 3:
		TargetWidget = Widget_HUDNameValueP4;
		break;
	default:
		return;
	}

	// if target color and current color is the same, skip
	if (TargetWidget->ColorAndOpacity.Equals(Color))
	{
		return;
	}

	TargetWidget->SetColorAndOpacity(Color);
}

void UHUDWidget::SetTimerValue(const float TimeLeft)
{
	const FText Text = UKismetTextLibrary::Conv_IntToText(UKismetMathLibrary::FFloor(TimeLeft));
	Widget_HUDNameValueTimer->Text_Value_Middle->SetText(Text);
}

ECountdownState UHUDWidget::SetPreGameCountdownState() const
{
	ECountdownState State;
	switch (ByteWarsGameState->GameStatus)
	{
	case EGameStatus::IDLE:
		State = ECountdownState::PRE;
		break;
	case EGameStatus::AWAITING_PLAYERS:
		State = ECountdownState::PRE;
		break;
	case EGameStatus::PRE_GAME_COUNTDOWN_STARTED:
		State = ECountdownState::COUNTING;
		break;
	case EGameStatus::GAME_STARTED:
		State = ECountdownState::POST;
		break;
	case EGameStatus::GAME_ENDS:
		State = ECountdownState::POST;
		break;
	case EGameStatus::INVALID:
		State = ECountdownState::POST;
		break;
	default:
		State = ECountdownState::POST;
	}
	return State;
}

int UHUDWidget::UpdatePreGameCountdownValue() const
{
	return UKismetMathLibrary::FFloor(ByteWarsGameState->PreGameCountdown);
}

void UHUDWidget::OnPreGameCountdownFinished()
{
}

ECountdownState UHUDWidget::SetNotEnoughPlayerCountdownState() const
{
	ECountdownState State;
	switch (ByteWarsGameState->GameStatus)
	{
	case EGameStatus::AWAITING_PLAYERS:
	case EGameStatus::AWAITING_PLAYERS_MID_GAME:
		State = ECountdownState::COUNTING;
		break;
	default:
		State = ECountdownState::INVALID;
	}
	return State;
}

int UHUDWidget::UpdateNotEnoughPlayerCountdownValue() const
{
	return UKismetMathLibrary::FFloor(ByteWarsGameState->NotEnoughPlayerCountdown);
}

void UHUDWidget::OnNotEnoughPlayerCountdownFinished()
{
}
