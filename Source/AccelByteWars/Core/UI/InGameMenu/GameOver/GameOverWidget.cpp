// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Core/UI/InGameMenu/GameOver/Components/GameOverLeaderboardEntry.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "CommonButtonBase.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"
#include "Kismet/KismetMathLibrary.h"

void UGameOverWidget::SetWinner(const FText& PlayerName, const FLinearColor& Color)
{
	Txt_Winner->SetText(PlayerName);
	Txt_Winner->SetColorAndOpacity(Color);
}

void UGameOverWidget::AddLeaderboardEntry(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor)
{
	const TWeakObjectPtr<UGameOverLeaderboardEntry> NewEntry = MakeWeakObjectPtr<UGameOverLeaderboardEntry>(CreateWidget<UGameOverLeaderboardEntry>(this, LeaderboardEntryClass.Get()));
	NewEntry->InitData(PlayerName, PlayerScore, PlayerKills, PlayerColor);
	Vb_Leaderboard->AddChild(NewEntry.Get());
}

void UGameOverWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ByteWarsGameState = GetWorld()->GetGameState<AAccelByteWarsGameStateBase>();

	// if on server, disable play again button
	if (GetOwningPlayer()->HasAuthority())
	{
		Btn_PlayAgain->SetVisibility(ESlateVisibility::Visible);
		Widget_Countdown->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		// on server
		Btn_PlayAgain->SetVisibility(ESlateVisibility::Collapsed);
		Widget_Countdown->SetVisibility(ESlateVisibility::Visible);
	}

	// Bind buttons click event.
	Btn_PlayAgain->OnClicked().AddUObject(this, &UGameOverWidget::PlayGameAgain);
	Btn_Quit->OnClicked().AddUObject(this, &UGameOverWidget::QuitGame);

	SetInputModeToUIOnly();

	// countdown setup
	Widget_Countdown->SetupWidget(
		FText::FromString(""),
		FText::FromString(""),
		FText::FromString("Quitting in:"));
	Widget_Countdown->CheckCountdownStateDelegate.BindUObject(this, &ThisClass::GetCountdownState);
	Widget_Countdown->UpdateCountdownValueDelegate.BindUObject(this, &ThisClass::GetCountdownValue);
	Widget_Countdown->OnCountdownFinishedDelegate.AddUObject(this, &ThisClass::OnCountdownFinished);
}

void UGameOverWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_PlayAgain->OnClicked().RemoveAll(this);
	Btn_Quit->OnClicked().RemoveAll(this);

	SetInputModeToGameOnly();
}

void UGameOverWidget::PlayGameAgain()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("GalaxyWorld"));
}

void UGameOverWidget::QuitGame()
{
	OnExitLevel();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

ECountdownState UGameOverWidget::GetCountdownState()
{
	ECountdownState CountdownState;
	switch (ByteWarsGameState->GameStatus)
	{
	case EGameStatus::GAME_ENDS:
		CountdownState = ECountdownState::COUNTING;
		break;
	case EGameStatus::INVALID:
		CountdownState = ECountdownState::POST;
		break;
	default:
		CountdownState = ECountdownState::PRE;;
	}
	return CountdownState;
}

int UGameOverWidget::GetCountdownValue()
{
	return UKismetMathLibrary::FFloor(ByteWarsGameState->PostGameCountdown);
}

void UGameOverWidget::OnCountdownFinished()
{
}
