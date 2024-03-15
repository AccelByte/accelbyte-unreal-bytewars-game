// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidget.h"

#include "HUDWidgetEntry.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ByteWarsGameState = GetWorld()->GetGameState<AAccelByteWarsInGameGameState>();

	// setup pre game countdown
	Widget_PreGameCountdown->SetupWidget(
		FText::FromString("Waiting for all players"),
		FText::FromString("Game Started"));
	Widget_PreGameCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetPreGameCountdownState);
	Widget_PreGameCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdatePreGameCountdownValue);
	OnPreGameCountdownFinishedDelegateHandle =
		Widget_PreGameCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnPreGameCountdownFinished);

	if (ByteWarsGameState->GameSetup.NotEnoughPlayerShutdownCountdown != INDEX_NONE &&
		ByteWarsGameState->GameSetup.MinimumTeamCountToPreventAutoShutdown != INDEX_NONE)
	{
		// setup not enough player countdown
		Widget_NotEnoughPlayerCountdown->SetupWidget(
			FText::FromString(""),
			FText::FromString(""),
			FText::FromString("Not enough players | Shutting down DS in: "),
			true);
		Widget_NotEnoughPlayerCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetNotEnoughPlayerCountdownState);
		Widget_NotEnoughPlayerCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdateNotEnoughPlayerCountdownValue);
		OnNotEnoughPlayerCountdownFinishedDelegateHandle =
			Widget_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnNotEnoughPlayerCountdownFinished);
	}
	else
	{
		Widget_NotEnoughPlayerCountdown->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Setup simulate crash countdown
	if (ByteWarsGameState->SimulateServerCrashCountdown != INDEX_NONE)
	{
		Widget_SimulateServerCrashCountdown->SetupWidget(
			FText::FromString(""),
			FText::FromString(""),
			FText::FromString("Simulating DS Crash in: "),
			true);
		Widget_SimulateServerCrashCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetSimulateServerCrashCountdownState);
		Widget_SimulateServerCrashCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdateSimulateServerCrashCountdownValue);
		OnSimulateServerCrashCountdownFinishedDelegateHandle =
			Widget_SimulateServerCrashCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnSimulateServerCrashCountdownFinished);
	}
	else
	{
		Widget_SimulateServerCrashCountdown->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Only show dedicated server FTUE on online session.
	if (FFTUEDialogueModel* FTUEDedicatedServer =
		FFTUEDialogueModel::GetMetadataById("ftue_ds_details", FTUEDialogues))
	{
#pragma region "Check if using AMS or not"
		bool bUseAMS = true; // default is true

		// Check launch param. Prioritize launch param.
		FString UseAMSString;
		if (FParse::Value(FCommandLine::Get(), TEXT("-bServerUseAMS="), UseAMSString))
		{
			bUseAMS = !UseAMSString.Equals("false", ESearchCase::Type::IgnoreCase);
		}
		// check DefaultEngine.ini next
		else
		{
			GConfig->GetBool(TEXT("/ByteWars/TutorialModule.DSEssentials"), TEXT("bServerUseAMS"), bUseAMS, GEngineIni);
		}
#pragma endregion

		// If using AMS then override the target URL 
		if (bUseAMS)
		{
			FTUEDedicatedServer->Button1.TargetURL = FString("{0}/ams/fleets-manager/any/server/{1}");
		}
	}
}

void UHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Widget_PreGameCountdown->OnCountdownFinishedDelegate.Remove(OnPreGameCountdownFinishedDelegateHandle);
	Widget_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.Remove(OnNotEnoughPlayerCountdownFinishedDelegateHandle);
}

bool UHUDWidget::SetValue(const FString Value, const int32 Index, const int32 BoxIndex)
{
	if (Index < 0 || Index > 3)
	{
		return false;
	}

	if (BoxIndex < 0 || BoxIndex > 2)
	{
		return false;
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
		return false;
	}

	if (!TargetWidget)
	{
		return false;
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
		return false;
	}

	if (!TargetTextBlock) 
	{
		return false;
	}

	TargetTextBlock->SetText(FText::FromString(Value));
	return true;
}

bool UHUDWidget::SetPowerUps(const TArray<TEnumAsByte<EPowerUpSelection>>& SelectedPowerUps, const TArray<int32>& PowerUpCounts, const int32 TeamIndex)
{
	if (TeamIndex < 0 || TeamIndex > 3)
	{
		return false;
	}

	UHUDWidgetEntry* TargetWidget;
	switch (TeamIndex)
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
		return false;
	}
	
	if (!TargetWidget) 
	{
		return false;
	}

	TargetWidget->SetPowerUpValues(SelectedPowerUps, PowerUpCounts);
	return true;
}

bool UHUDWidget::SetColorChecked(const int32 Index, const FLinearColor Color)
{
	if (Index < 0 || Index > 3)
	{
		return false;
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
		return false;
	}

	if (!TargetWidget) 
	{
		return false;
	}

	// Change the color if not the same.
	if (!TargetWidget->ColorAndOpacity.Equals(Color))
	{
		TargetWidget->SetColorAndOpacity(Color);
	}

	return true;
}

bool UHUDWidget::ToggleEntry(const int32 Index, const bool bActivate)
{
	if (Index < 0 || Index > 3)
	{
		return false;
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
		return false;
	}

	if (!TargetWidget) 
	{
		return false;
	}

	TargetWidget->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	return true;
}

void UHUDWidget::SetTimerValue(const float TimeLeft)
{
	const FText Text = UKismetTextLibrary::Conv_IntToText(UKismetMathLibrary::FFloor(TimeLeft));
	Widget_HUDNameValueTimer->Text_Value_Middle->SetText(Text);
}

void UHUDWidget::GetVisibleHUDPixelPosition(FVector2D& OutMinPixelPosition, FVector2D& OutMaxPixelPosition) const
{
	FVector2D ViewportPosition;

	const FGeometry& CachedGeometry = Widget_VisibleBorder->GetCachedGeometry();
	const FVector2D& CachedGeometrySize = CachedGeometry.GetLocalSize();

	USlateBlueprintLibrary::LocalToViewport(
		GetWorld(),
		CachedGeometry,
		FVector2D::Zero(),
		OutMinPixelPosition,
		ViewportPosition);

	OutMaxPixelPosition.X = OutMinPixelPosition.X + CachedGeometrySize.X;
	OutMaxPixelPosition.Y = OutMinPixelPosition.Y + CachedGeometrySize.Y;

	return;
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
	default:
		State = ECountdownState::INVALID;
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

ECountdownState UHUDWidget::SetSimulateServerCrashCountdownState() const
{
	ECountdownState State;
	switch (ByteWarsGameState->GameStatus)
	{
	case EGameStatus::AWAITING_PLAYERS_MID_GAME:
	case EGameStatus::GAME_STARTED:
		State = ECountdownState::COUNTING;
		break;
	default:
		State = ECountdownState::INVALID;
	}
	return State;
}

int UHUDWidget::UpdateSimulateServerCrashCountdownValue() const
{
	return UKismetMathLibrary::FFloor(ByteWarsGameState->SimulateServerCrashCountdown);
}

void UHUDWidget::OnSimulateServerCrashCountdownFinished()
{
}
