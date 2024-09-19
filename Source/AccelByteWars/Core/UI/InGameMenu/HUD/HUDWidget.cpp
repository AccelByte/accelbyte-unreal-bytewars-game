// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidget.h"

#include "HUDWidgetEntry.h"
#include "HUDKillFeedWidgetEntry.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

// defines for spectating text effect
#define FADE_DURATION 2 // Single fade in/out duration
#define TOTAL_DURATION (2 * FADE_DURATION) // Total duration per cycle (n second fade out n second fade in )
#define MIN_OPACITY 0.3f
#define OPACITY_RANGE (1 - MIN_OPACITY)
#define NORMALIZED_MID_VALUE 0.5f

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

	// Setup HUD update callback
	ByteWarsGameState->OnTeamsChanged.AddUObject(this, &ThisClass::UpdateHUD);
	ByteWarsGameState->OnPowerUpChanged.AddUObject(this, &ThisClass::UpdateHUD);
	UpdateHUD();
	
	ByteWarsGameState->OnTeamsChanged.AddUObject(this, &ThisClass::CheckSpectatingText);

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

	Text_Spectating->SetVisibility(ESlateVisibility::Collapsed);

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

	AAccelByteWarsInGameGameState::OnPlayerDieDelegate.AddUObject(this, &ThisClass::OnPlayerDie);
}

void UHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Widget_PreGameCountdown->OnCountdownFinishedDelegate.Remove(OnPreGameCountdownFinishedDelegateHandle);
	Widget_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.Remove(OnNotEnoughPlayerCountdownFinishedDelegateHandle);
	ByteWarsGameState->OnTeamsChanged.RemoveAll(this);
	ByteWarsGameState->OnPowerUpChanged.RemoveAll(this);

	AAccelByteWarsInGameGameState::OnPlayerDieDelegate.RemoveAll(this);
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	SetTimerValue(ByteWarsGameState->TimeLeft);
	UpdateSpectatingTextEffect(InDeltaTime);
}

void UHUDWidget::UpdateHUD()
{
	for (const FGameplayTeamData& TeamData : ByteWarsGameState->Teams)
	{
		// Toggle entry
		if (TeamData.TeamMembers.IsEmpty())
		{
			ToggleEntry(TeamData.TeamId, false);
			continue;
		}
		ToggleEntry(TeamData.TeamId, true);

		// Set color
		UGameInstance* GameInstance = GetGameInstance();
		if (!GameInstance)
		{
			return;
		}
		UAccelByteWarsGameInstance* AbGameInstance = Cast<UAccelByteWarsGameInstance>(GameInstance);
		if (!AbGameInstance)
		{
			return;
		}

		if (!SetColorChecked(TeamData.TeamId, AbGameInstance->GetTeamColor(TeamData.TeamId)))
		{
			return;
		}

		// Update lives left, score, and kill count
		SetValue(FString::FromInt(TeamData.GetTeamLivesLeft()), TeamData.TeamId, 0);
		SetValue(FString::FromInt(TeamData.GetTeamScore()), TeamData.TeamId, 1);
		SetValue(FString::FromInt(TeamData.GetTeamKillCount()), TeamData.TeamId, 2);

		// Update PowerUps
		for (int i = 0; i < TeamData.TeamMembers.Num(); ++i)
		{
			UpdatePowerUpDisplay(TeamData.TeamMembers[i], i);
		}
	}
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

bool UHUDWidget::SetValue(const FString Value, const int32 TeamIndex, const int32 BoxIndex)
{
	if (TeamIndex < 0 || TeamIndex > 3)
	{
		return false;
	}

	if (BoxIndex < 0 || BoxIndex > 2)
	{
		return false;
	}

	const UHUDWidgetEntry* TargetWidget;
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

	UTextBlock* TargetTextBlock;
	switch (BoxIndex)
	{
	case 0:
		// Lives left
		TargetTextBlock = TargetWidget->Text_Value_Left;
		break;
	case 1:
		// Score
		TargetTextBlock = TargetWidget->Text_Value_Middle;
		break;
	case 2:
		// Kills
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

void UHUDWidget::SetTimerValue(const float TimeLeft)
{
	const FText Text = UKismetTextLibrary::Conv_IntToText(UKismetMathLibrary::FFloor(TimeLeft));
	Widget_HUDNameValueTimer->Text_Value_Middle->SetText(Text);
}

void UHUDWidget::UpdatePowerUpDisplay(const FGameplayPlayerData& PlayerData, const int32 TeamMemberIndex) const
{
	// Get player state
	APlayerState* PlayerState = nullptr;
	for (const TObjectPtr<APlayerState> SearchPlayerState : ByteWarsGameState->PlayerArray)
	{
		// construct player's identification
		int32 ControllerId = INDEX_NONE;
		if (const APlayerController* PC = SearchPlayerState->GetPlayerController())
		{
			if (const ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				ControllerId = LP->GetLocalPlayerIndex();
			}
		}
		FGameplayPlayerData SearchPlayerData{SearchPlayerState->GetUniqueId(), ControllerId};
		if (PlayerData == SearchPlayerData)
		{
			PlayerState = SearchPlayerState;
			break;
		}
	}
	if (!PlayerState)
	{
		return;
	}

	// get equipped power up
	AAccelByteWarsPlayerState* AbPs = Cast<AAccelByteWarsPlayerState>(PlayerState);
	if (!AbPs)
	{
		return;
	}
	const FEquippedItem EquippedItem = AbPs->GetEquippedItem(EItemType::PowerUp, true);
	if (EquippedItem.ItemId.IsEmpty())
	{
		return;
	}

	// update widget
	UHUDWidgetEntry* TargetWidget = nullptr;
	switch (AbPs->TeamId)
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
	}
	TargetWidget->SetPowerUpValues(EquippedItem.ItemId, EquippedItem.Count, TeamMemberIndex);
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

void UHUDWidget::OnPlayerDie(const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer)
{
	if (!ByteWarsGameState)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot handle on-player die event. Game state is null."));
		return;
	}

	FGameplayPlayerData* KillerPlayerData =
		ByteWarsGameState->GetPlayerDataById(Killer->GetUniqueId(), AccelByteWarsUtility::GetControllerId(Killer));

	FGameplayPlayerData* VictimPlayerData =
		ByteWarsGameState->GetPlayerDataById(DeathPlayer->GetUniqueId(), AccelByteWarsUtility::GetControllerId(DeathPlayer));

	if (!KillerPlayerData || !VictimPlayerData)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot handle on-player die event. Player data is null."));
		return;
	}

	TWeakObjectPtr<UHUDKillFeedData> KillFeed = NewObject<UHUDKillFeedData>();
	KillFeed->Killer = *KillerPlayerData;
	KillFeed->Victim = *VictimPlayerData;

	W_KillFeed->PushNotification(KillFeed.Get());
}

void UHUDWidget::UpdateSpectatingTextEffect(float DeltaTime)
{
	// will make text effect fade out and fade in, each for FADE_DURATION second
	if(Text_Spectating->GetVisibility() == ESlateVisibility::Visible)
	{
		SpectatingTextVisibleRunningTime += DeltaTime;		

		// Fractional value is in range 0 < Fractional < 1
		// ChangeFactor should have similar value for Fraction time 't' and '1.0 - t', and within range 0 < ChangeFactor < 1
		const float Fractional = FMath::Frac(SpectatingTextVisibleRunningTime / TOTAL_DURATION);
		const float ChangeFactor = ((Fractional < NORMALIZED_MID_VALUE ? 1 - Fractional : Fractional) - NORMALIZED_MID_VALUE) / NORMALIZED_MID_VALUE;
		const float Opacity = MIN_OPACITY + (ChangeFactor * OPACITY_RANGE);
		Text_Spectating->SetOpacity(Opacity);
	}
}

void UHUDWidget::CheckSpectatingText()
{
	// check local player remaining lives, and show spectating text if no local player do not have remaining live
	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if(ABGameState && !ABGameState->HasGameEnded())
	{
		const APlayerController* LocalPlayerController =  GetGameInstance()->GetFirstLocalPlayerController();
		if(LocalPlayerController != nullptr &&  LocalPlayerController->PlayerState != nullptr
			&& Text_Spectating->GetVisibility() != ESlateVisibility::Visible)
		{
			// check using FGameplayPlayerData because num lives from player state have wrong value
			const FGameplayPlayerData* PlayerData = ABGameState->GetPlayerDataById(LocalPlayerController->PlayerState->GetUniqueId());
			if(PlayerData && PlayerData->NumLivesLeft <=0)
			{
				Text_Spectating->SetVisibility(ESlateVisibility::Visible);
				SpectatingTextVisibleRunningTime = 0.0f;
			}
		}
	}
}

#undef FADE_DURATION
#undef TOTAL_DURATION
#undef MIN_OPACITY
#undef OPACITY_RANGE
#undef NORMALIZED_MID_VALUE