// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Core/UI/InGameMenu/GameOver/Components/GameOverLeaderboardEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Kismet/KismetMathLibrary.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

UGameOverLeaderboardEntry* UGameOverWidget::AddLeaderboardEntry(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor)
{
	const TWeakObjectPtr<UGameOverLeaderboardEntry> NewEntry = MakeWeakObjectPtr<UGameOverLeaderboardEntry>(CreateWidget<UGameOverLeaderboardEntry>(this, LeaderboardEntryClass.Get()));
	Vb_Leaderboard->AddChild(NewEntry.Get());
	NewEntry->InitData(PlayerName, PlayerScore, PlayerKills, PlayerColor);

	return NewEntry.Get();
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
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
	if (GameState->GameSetup.GameEndsShutdownCountdown != INDEX_NONE)
	{
		Widget_Countdown->SetupWidget(
			FText::FromString(""),
			FText::FromString(""),
			FText::FromString("Quitting in:"));
		Widget_Countdown->CheckCountdownStateDelegate.BindUObject(this, &UGameOverWidget::GetCountdownState);
		Widget_Countdown->UpdateCountdownValueDelegate.BindUObject(this, &UGameOverWidget::GetCountdownValue);
		Widget_Countdown->OnCountdownFinishedDelegate.AddUObject(this, &UGameOverWidget::OnCountdownFinished);
	}
	else
	{
		Widget_Countdown->SetVisibility(ESlateVisibility::Collapsed);
	}
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
	bool bIsWinnerLocalPlayer = false;

	// Generate leaderboard entries.
	const ENetMode NetMode = GetOwningPlayer()->GetNetMode();
	const FUniqueNetIdRepl LocalPlayerNetId = GetGameInstance()->GetPrimaryPlayerUniqueIdRepl();
	int32 PlayerIndex = 0;
	for (const FGameplayTeamData& Team : GameState->Teams) 
	{
		if (Team.TeamMembers.IsEmpty()) continue;

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

			FString PlayerName = FString::Printf(TEXT("Player %d"), PlayerIndex);
			if (!Member.PlayerName.IsEmpty())
			{
				PlayerName = Member.PlayerName;
			}

			if (Team.TeamId == WinnerTeamId) 
			{
				WinnerPlayerName = PlayerName;
				bIsWinnerLocalPlayer = LocalPlayerNetId.IsValid() && LocalPlayerNetId == Member.UniqueNetId && NetMode != ENetMode::NM_Standalone;
			}

			AddLeaderboardEntry(FText::FromString(PlayerName), Member.Score, Member.KillCount, TeamColor);
		}
	}

	// Display the winner.
	const bool bIsDraw = WinnerTeamId == INDEX_NONE;
	if (bIsDraw)
	{
		Txt_Winner->SetText(LOCTEXT("Game over draw", "Game Draw!"));
		Txt_Winner->SetColorAndOpacity(FLinearColor::White);
	}
	else 
	{
		const FString WinnerName = GameState->GameSetup.bIsTeamGame ? FString::Printf(TEXT("Team %d"), WinnerTeamId + 1) : WinnerPlayerName;
		const FText WinnerLabel = bIsWinnerLocalPlayer ?
			LOCTEXT("Game over local player wins", "You Win!") :
			FText::Format(LOCTEXT("Game over other player wins", "{0} Wins!"), FFormatOrderedArguments { FFormatArgumentValue(FText::FromString(WinnerName)) });

		Txt_Winner->SetText(WinnerLabel);
		Txt_Winner->SetColorAndOpacity(GameInstance->GetTeamColor(WinnerTeamId));
	}
}

void UGameOverWidget::PlayGameAgain()
{
	if (const AAccelByteWarsGameMode* GameMode = Cast<AAccelByteWarsGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->DelayedServerTravel("/Game/ByteWars/Maps/GalaxyWorld/GalaxyWorld");
	}
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

#undef LOCTEXT_NAMESPACE