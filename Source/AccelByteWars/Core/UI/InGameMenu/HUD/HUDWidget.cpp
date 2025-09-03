// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidget.h"

#include "HUDWidgetEntry.h"
#include "HUDKillFeedWidgetEntry.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/Player/AccelByteWarsInGamePlayerController.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
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

	Btn_Pause->OnClicked().AddUObject(this, &ThisClass::Pause);
	InitMobileControls();

	GameState = GetWorld()->GetGameState<AAccelByteWarsInGameGameState>();
	ensure(GameState);

	W_PreGameCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetPreGameCountdownState);
	W_PreGameCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdatePreGameCountdownValue);
	OnPreGameCountdownFinishedDelegateHandle =
		W_PreGameCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnPreGameCountdownFinished);

	W_NotEnoughPlayerCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetNotEnoughPlayerCountdownState);
	W_NotEnoughPlayerCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdateNotEnoughPlayerCountdownValue);
	OnNotEnoughPlayerCountdownFinishedDelegateHandle =
		W_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnNotEnoughPlayerCountdownFinished);

	// Setup HUD update callback
	GameState->OnTeamsChanged.AddUObject(this, &ThisClass::UpdateHUDEntries);
	GameState->OnPowerUpChanged.AddUObject(this, &ThisClass::UpdateHUDEntries);
	
	GameState->OnTeamsChanged.AddUObject(this, &ThisClass::CheckSpectatingText);

	// Setup simulate crash countdown
	W_SimulateServerCrashCountdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::SetSimulateServerCrashCountdownState);
	W_SimulateServerCrashCountdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::UpdateSimulateServerCrashCountdownValue);
	OnSimulateServerCrashCountdownFinishedDelegateHandle =
		W_SimulateServerCrashCountdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnSimulateServerCrashCountdownFinished);

	Tb_Spectating->SetVisibility(ESlateVisibility::Collapsed);

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

	Btn_Pause->OnClicked().Clear();
	ResetMobileControls();

	W_PreGameCountdown->OnCountdownFinishedDelegate.Remove(OnPreGameCountdownFinishedDelegateHandle);
	W_NotEnoughPlayerCountdown->OnCountdownFinishedDelegate.Remove(OnNotEnoughPlayerCountdownFinishedDelegateHandle);
	GameState->OnTeamsChanged.RemoveAll(this);
	GameState->OnPowerUpChanged.RemoveAll(this);

	AAccelByteWarsInGameGameState::OnPlayerDieDelegate.RemoveAll(this);
}

void UHUDWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// setup pre game countdown
	W_PreGameCountdown->SetVisibility(ESlateVisibility::Visible);
	W_PreGameCountdown->SetupWidget(
		FText::FromString("Waiting for all players"),
		FText::FromString("Game Started"));

	if (GameState->GameSetup.NotEnoughPlayerShutdownCountdown != INDEX_NONE && GameState->GameSetup.MinimumTeamCountToPreventAutoShutdown != INDEX_NONE)
	{
		// setup not enough player countdown
		W_NotEnoughPlayerCountdown->SetupWidget(
			FText::FromString(""),
			FText::FromString(""),
			FText::FromString("Not enough players | Shutting down DS in: "),
			true);
		W_NotEnoughPlayerCountdown->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		W_NotEnoughPlayerCountdown->SetVisibility(ESlateVisibility::Collapsed);
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
	if (GameState->SimulateServerCrashCountdown != static_cast<float>(INDEX_NONE))
#else
	if (GameState->SimulateServerCrashCountdown != INDEX_NONE)
#endif
	{
		W_SimulateServerCrashCountdown->SetupWidget(
			FText::FromString(""),
			FText::FromString(""),
			FText::FromString("Simulating DS Crash in: "),
			true);
		W_SimulateServerCrashCountdown->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		W_SimulateServerCrashCountdown->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!HUDWidgetEntries.IsEmpty())
	{
		Hb_LeftPanel->ClearChildren();
		Hb_RightPanel->ClearChildren();
		HUDWidgetEntries.Empty();
	}
	UpdateHUDEntries();
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	SetTimerValue(GameState->TimeLeft);
	UpdateSpectatingTextEffect(InDeltaTime);
}

void UHUDWidget::GenerateHUDEntries()
{
	Hb_LeftPanel->ClearChildren();
	Hb_RightPanel->ClearChildren();

	// Temporary arrays to hold the teams for left and right panels.
	TArray<TWeakObjectPtr<UHUDWidgetEntry>> LeftWidgetEntries {};
	TArray<TWeakObjectPtr<UHUDWidgetEntry>> RightWidgetEntries {};

	int32 Index = 0;
	for (const FGameplayTeamData& Team : GameState->Teams)
	{
		if (Team.TeamMembers.IsEmpty())
		{
			continue;
		}

		// Create the widget and store it to the cache.
		const TWeakObjectPtr<UHUDWidgetEntry> WidgetEntry = 
			MakeWeakObjectPtr<UHUDWidgetEntry>(CreateWidget<UHUDWidgetEntry>(this, HUDWidgetEntryClass.Get()));
		HUDWidgetEntries.Add(Team.TeamId, WidgetEntry);

		// Add the entry to the left or right side based on the index.
		if (++Index % 2 == 0)
		{
			RightWidgetEntries.Add(WidgetEntry);
		}
		else
		{
			LeftWidgetEntries.Add(WidgetEntry);
		}
	}

	// Add the right entries in normal order.
	for (TWeakObjectPtr<UHUDWidgetEntry> WidgetEntry : RightWidgetEntries)
	{
		Hb_RightPanel->AddChild(WidgetEntry.Get());
	}

	// Add the left entries in reversed order.
	for (int32 i = LeftWidgetEntries.Num() - 1; i >= 0; --i)
	{
		Hb_LeftPanel->AddChild(LeftWidgetEntries[i].Get());
	}
}

void UHUDWidget::UpdateHUDEntries()
{
	for (const FGameplayTeamData& Team : GameState->Teams)
	{
		if (Team.TeamMembers.IsEmpty()) 
		{
			continue;
		}

		/* If the entry for the team does not exist, regenerate the HUD entries.
		 * This also guarantees the next teams have their entries, so this checker will be called only once. */
		if (!HUDWidgetEntries.Contains(Team.TeamId))
		{
			GenerateHUDEntries();
		}

		HUDWidgetEntries[Team.TeamId]->Init(Team);
	}
}

void UHUDWidget::GetVisibleHUDPixelPosition(FVector2D& OutMinPixelPosition, FVector2D& OutMaxPixelPosition) const
{
	FVector2D ViewportPosition;

	const FGeometry& CachedGeometry = W_VisibleBorder->GetCachedGeometry();
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

void UHUDWidget::SetTimerValue(const float TimeLeft)
{
	Tb_Timer->SetText(UKismetTextLibrary::Conv_IntToText(UKismetMathLibrary::FFloor(TimeLeft)));
}

ECountdownState UHUDWidget::SetPreGameCountdownState() const
{
	ECountdownState State;
	switch (GameState->GameStatus)
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
	return UKismetMathLibrary::FFloor(GameState->PreGameCountdown);
}

void UHUDWidget::OnPreGameCountdownFinished()
{
}

ECountdownState UHUDWidget::SetNotEnoughPlayerCountdownState() const
{
	ECountdownState State;
	switch (GameState->GameStatus)
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
	return UKismetMathLibrary::FFloor(GameState->NotEnoughPlayerCountdown);
}

void UHUDWidget::OnNotEnoughPlayerCountdownFinished()
{
}

ECountdownState UHUDWidget::SetSimulateServerCrashCountdownState() const
{
	ECountdownState State;
	switch (GameState->GameStatus)
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
	return UKismetMathLibrary::FFloor(GameState->SimulateServerCrashCountdown);
}

void UHUDWidget::OnSimulateServerCrashCountdownFinished()
{
}

void UHUDWidget::OnPlayerDie(const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer)
{
	if (!GameState)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot handle on-player die event. Game state is null."));
		return;
	}
	if (!Killer)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Player died, but not because other player."));
		return;
	}

	FGameplayPlayerData* KillerPlayerData =
		GameState->GetPlayerDataById(Killer->GetUniqueId(), AccelByteWarsUtility::GetControllerId(Killer));

	FGameplayPlayerData* VictimPlayerData =
		GameState->GetPlayerDataById(DeathPlayer->GetUniqueId(), AccelByteWarsUtility::GetControllerId(DeathPlayer));

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
	if(Tb_Spectating->GetVisibility() == ESlateVisibility::Visible)
	{
		SpectatingTextVisibleRunningTime += DeltaTime;		

		// Fractional value is in range 0 < Fractional < 1
		// ChangeFactor should have similar value for Fraction time 't' and '1.0 - t', and within range 0 < ChangeFactor < 1
		const float Fractional = FMath::Frac(SpectatingTextVisibleRunningTime / TOTAL_DURATION);
		const float ChangeFactor = ((Fractional < NORMALIZED_MID_VALUE ? 1 - Fractional : Fractional) - NORMALIZED_MID_VALUE) / NORMALIZED_MID_VALUE;
		const float Opacity = MIN_OPACITY + (ChangeFactor * OPACITY_RANGE);
		Tb_Spectating->SetOpacity(Opacity);
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
			&& Tb_Spectating->GetVisibility() != ESlateVisibility::Visible)
		{
			// check using FGameplayPlayerData because num lives from player state have wrong value
			const FGameplayPlayerData* PlayerData = ABGameState->GetPlayerDataById(LocalPlayerController->PlayerState->GetUniqueId());
			if(PlayerData && PlayerData->NumLivesLeft <=0)
			{
				Tb_Spectating->SetVisibility(ESlateVisibility::Visible);
				SpectatingTextVisibleRunningTime = 0.0f;
			}
		}
	}
}

void UHUDWidget::Pause()
{
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		if (UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget())) 
		{
			BaseUIWidget->PushWidgetToStack(EBaseUIStackType::InGameMenu, PauseWidgetClass);
		}
	}
}

#pragma region Mobile Controls
void UHUDWidget::InitMobileControls()
{
	// Only enable use power up button if there is power up equipped.
	Btn_UsePowerUpMobile->SetVisibility(ESlateVisibility::Collapsed);
	if (AAccelByteWarsInGamePlayerController* PC = Cast<AAccelByteWarsInGamePlayerController>(GetOwningPlayer()))
	{
		AAccelByteWarsPlayerState* PlayerState = Cast<AAccelByteWarsPlayerState>(PC->PlayerState);
		if (PlayerState)
		{
			const FEquippedItem PowerUp = PlayerState->GetEquippedItem(EItemType::PowerUp, true);
			Btn_UsePowerUpMobile->SetVisibility(PowerUp.ItemId.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		}
	}

	Btn_PowerUpMobile->OnHold.AddUObject(this, &ThisClass::AdjustPowerMobile, 1.0f);
	Btn_PowerDownMobile->OnHold.AddUObject(this, &ThisClass::AdjustPowerMobile, -1.0f);
	Btn_PowerUpMobile->OnReleased.AddUObject(this, &ThisClass::AdjustPowerMobile, 0.0f);
	Btn_PowerDownMobile->OnReleased.AddUObject(this, &ThisClass::AdjustPowerMobile, 0.0f);

	Btn_RotateRightMobile->OnPressed.AddUObject(this, &ThisClass::RotateShipMobile, 1.0f);
	Btn_RotateLeftMobile->OnPressed.AddUObject(this, &ThisClass::RotateShipMobile, -1.0f);
	Btn_RotateRightMobile->OnReleased.AddUObject(this, &ThisClass::RotateShipMobile, 0.0f);
	Btn_RotateLeftMobile->OnReleased.AddUObject(this, &ThisClass::RotateShipMobile, 0.0f);

	Btn_FireMissileMobile->OnClicked().AddUObject(this, &ThisClass::FireMissileMobile);
	Btn_UsePowerUpMobile->OnClicked().AddUObject(this, &ThisClass::UsePowerUpMobile);
}

void UHUDWidget::ResetMobileControls()
{
	Btn_PowerUpMobile->OnHold.Clear();
	Btn_PowerDownMobile->OnHold.Clear();
	Btn_PowerUpMobile->OnReleased.Clear();
	Btn_PowerDownMobile->OnReleased.Clear();

	Btn_RotateRightMobile->OnPressed.Clear();
	Btn_RotateLeftMobile->OnPressed.Clear();
	Btn_RotateRightMobile->OnReleased.Clear();
	Btn_RotateLeftMobile->OnReleased.Clear();

	Btn_FireMissileMobile->OnClicked().Clear();
	Btn_UsePowerUpMobile->OnClicked().Clear();
}

void UHUDWidget::RotateShipMobile(const float Value)
{
	if (AAccelByteWarsInGamePlayerController* PC = Cast<AAccelByteWarsInGamePlayerController>(GetOwningPlayer()))
	{
		PC->RotateShip(Value);
	}
}

void UHUDWidget::AdjustPowerMobile(const float Value)
{
	if (AAccelByteWarsInGamePlayerController* PC = Cast<AAccelByteWarsInGamePlayerController>(GetOwningPlayer()))
	{
		PC->AdjustPower(Value);
	}
}

void UHUDWidget::FireMissileMobile()
{
	if (AAccelByteWarsInGamePlayerController* PC = Cast<AAccelByteWarsInGamePlayerController>(GetOwningPlayer()))
	{
		PC->FireMissile();
	}
}

void UHUDWidget::UsePowerUpMobile()
{
	if (AAccelByteWarsInGamePlayerController* PC = Cast<AAccelByteWarsInGamePlayerController>(GetOwningPlayer()))
	{
		PC->UsePowerUp();
	}
}

#pragma endregion

#undef FADE_DURATION
#undef TOTAL_DURATION
#undef MIN_OPACITY
#undef OPACITY_RANGE
#undef NORMALIZED_MID_VALUE