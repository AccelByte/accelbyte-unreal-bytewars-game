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
	Btn_PlayAgain->SetVisibility(
		GetOwningPlayer()->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Bind buttons click event.
	Btn_PlayAgain->OnClicked().AddUObject(this, &UGameOverWidget::PlayGameAgain);
	Btn_Quit->OnClicked().AddUObject(this, &UGameOverWidget::QuitGame);

	SetupLeaderboard();

	SetInputModeToUIOnly();
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
	int HighestScore = 0;
	int WinnerTeamId = INDEX_NONE;

	TArray<FUniqueNetIdPtr> PlayerNetIds;
	TArray<UGameOverLeaderboardEntry*> LeaderboardEntries;
	for (FGameplayTeamData Team : GameState->Teams) 
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
		}

		const FLinearColor TeamColor = GameInstance->GetTeamColor(Team.TeamId);

		// Generate team members entry.
		for (FGameplayPlayerData Member : Team.TeamMembers) 
		{
			// Save player net id. It will be used to get player's information (username, etc) from backend.
			const FUniqueNetIdPtr PlayerNetId = Member.UniqueNetId.GetUniqueNetId();
			if (PlayerNetId.IsValid()) 
			{
				PlayerNetIds.Add(PlayerNetId);
			}

			UGameOverLeaderboardEntry* NewEntry = AddLeaderboardEntry(
				FText::FromString(FString::Printf(TEXT("Player %d"), LeaderboardEntries.Num() + 1)),
				Member.Score,
				Member.KillCount,
				TeamColor);

			LeaderboardEntries.Add(NewEntry);
		}
	}

	// Check if game running online, if yes then update the leaderboard.
	if (GetOwningPlayer()->GetNetMode() != ENetMode::NM_Standalone) 
	{
		OnGenerateGameOverLeaderboard.ExecuteIfBound(PlayerNetIds, LeaderboardEntries);
	}

	// If single player, the first team always wins.
	if (GameState->Teams.Num() == 1) 
	{
		WinnerTeamId = 0;
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
		SetWinner(
			FText::FromString(FString::Printf(TEXT("Team %d"), WinnerTeamId + 1)),
			GameInstance->GetTeamColor(WinnerTeamId)
		);
	}
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