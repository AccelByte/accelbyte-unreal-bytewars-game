// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Core/UI/InGameMenu/GameOver/Components/GameOverLeaderboardEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Kismet/KismetMathLibrary.h"

void UGameOverWidget::SetWinner(const FText& PlayerName, const FLinearColor& Color)
{
	Txt_Winner->SetText(PlayerName);
	Txt_Winner->SetColorAndOpacity(Color);
}

UGameOverLeaderboardEntry* UGameOverWidget::AddLeaderboardEntry(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor)
{
	const TWeakObjectPtr<UGameOverLeaderboardEntry> NewEntry = MakeWeakObjectPtr<UGameOverLeaderboardEntry>(CreateWidget<UGameOverLeaderboardEntry>(this, LeaderboardEntryClass.Get()));
	NewEntry->InitData(PlayerName, PlayerScore, PlayerKills, PlayerColor);
	Vb_Leaderboard->AddChild(NewEntry.Get());

	return NewEntry.Get();
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	GameState = Cast<AAccelByteWarsGameStateBase>(GetWorld()->GetGameState());
	ensure(GameState);
}

void UGameOverWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

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

	SetupLeaderboard();

	SetInputModeToUIOnly();

	// countdown setup
	Widget_Countdown->SetupWidget(
		FText::FromString(""),
		FText::FromString(""),
		FText::FromString("Quitting in:"));
	Widget_Countdown->CheckCountdownStateDelegate.BindUObject(this, &UGameOverWidget::GetCountdownState);
	Widget_Countdown->UpdateCountdownValueDelegate.BindUObject(this, &UGameOverWidget::GetCountdownValue);
	Widget_Countdown->OnCountdownFinishedDelegate.AddUObject(this, &UGameOverWidget::OnCountdownFinished);
}

void UGameOverWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_PlayAgain->OnClicked().RemoveAll(this);
	Btn_Quit->OnClicked().RemoveAll(this);

	SetInputModeToGameOnly();
}

void UGameOverWidget::SetupLeaderboard()
{
	int32 HighestScore = INDEX_NONE;
	int32 WinnerTeamId = INDEX_NONE;
	FString WinnerPlayerName = TEXT("");

	// Generate leaderboard entries.
	int32 PlayerIndex = 0;
	for (const FGameplayTeamData& Team : GameState->Teams) 
	{
		// Get team with highest score.
		if (Team.GetTeamScore() > HighestScore) 
		{
			HighestScore = Team.GetTeamScore();
			WinnerTeamId = Team.TeamId;
		}
		// No winner, draw.
		else if (Team.GetTeamScore() == HighestScore)
		{
			WinnerTeamId = INDEX_NONE;
			WinnerPlayerName = TEXT("");
		}

		const FLinearColor TeamColor = GameInstance->GetTeamColor(Team.TeamId);

		// Generate team members entry.
		for (const FGameplayPlayerData& Member : Team.TeamMembers) 
		{
			PlayerIndex++;
			const FString PlayerName = Member.PlayerName.IsEmpty() ? FString::Printf(TEXT("Player %d"), PlayerIndex) : Member.PlayerName;
			if (Team.TeamId == WinnerTeamId) 
			{
				WinnerPlayerName = PlayerName;
			}

			AddLeaderboardEntry(FText::FromString(PlayerName), Member.Score, Member.KillCount, TeamColor);
		}
	}

	// Display draw game.
	if (WinnerTeamId == INDEX_NONE) 
	{
		Ws_Winner->SetActiveWidgetIndex(1);
	}
	// Display winner team.
	else 
	{
		Ws_Winner->SetActiveWidgetIndex(0);

		const FString WinnerName = (GameState->GameModeType == EGameModeType::TDM) ? FString::Printf(TEXT("Team %d"), WinnerTeamId + 1) : WinnerPlayerName;
		SetWinner(FText::FromString(WinnerName), GameInstance->GetTeamColor(WinnerTeamId));
	}
}

void UGameOverWidget::PlayGameAgain()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("GalaxyWorld"));
}

void UGameOverWidget::QuitGame()
{
	if (OnQuitGameDelegate.IsBound())
	{
		OnQuitGameDelegate.Broadcast(GetOwningPlayer());
	}

	OnExitLevel();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

ECountdownState UGameOverWidget::GetCountdownState()
{
	ECountdownState CountdownState;
	switch (GameState->GameStatus)
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
	return UKismetMathLibrary::FFloor(GameState->PostGameCountdown);
}

void UGameOverWidget::OnCountdownFinished()
{
}