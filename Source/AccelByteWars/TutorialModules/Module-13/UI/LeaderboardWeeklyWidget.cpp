// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/UI/LeaderboardWeeklyWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardsWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"

void ULeaderboardWeeklyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PeriodicLeaderboardSubsystem = GameInstance->GetSubsystem<UPeriodicBoardSubsystem>();
	ensure(PeriodicLeaderboardSubsystem);
}

void ULeaderboardWeeklyWidget::NativeOnActivated()
{
	// Set leaderboard code based on board-unreal-highestscore-{gamemode} format. 
	LeaderboardCode = FString::Printf(TEXT("board-unreal-highestscore-%s"), *ULeaderboardsWidget::GetLeaderboardGameMode());

	// Set cycle id to the weekly leaderboard’s cycle id.
	CycleId = FString("55c5b1d5a4e14eaba45477cd70de9c15");

	if (FFTUEDialogueModel* FTUELeaderboard = FFTUEDialogueModel::GetMetadataById("ftue_weekly_leaderboard"))
	{
		FTUELeaderboard->Button1.URLArguments[0].Argument = LeaderboardCode;
		FTUELeaderboard->Button1.URLArguments[1].Argument = CycleId;
	}

	Super::NativeOnActivated();

	// Reset widgets.
	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_Leaderboard->ClearListItems();

	// Get leaderboard weekly rankings.
	GetWeeklyRankings();
}

void ULeaderboardWeeklyWidget::GetWeeklyRankings()
{
	FUniqueNetIdRepl PlayerNetId = GetOwningPlayer()->GetLocalPlayer()->GetPreferredUniqueNetId();
	if (!PlayerNetId.IsValid())
	{
		return;
	}

	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	PeriodicLeaderboardSubsystem->GetPeriodicRankings(
		GetOwningPlayer(),
		LeaderboardCode,
		CycleId,
		ResultLimit,
		FOnGetLeaderboardRankingComplete::CreateWeakLambda(this, [this, PlayerNetId](bool bWasSuccessful, const TArray<ULeaderboardRank*> Rankings)
		{
			if (!bWasSuccessful)
			{
				Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
				return;
			}

			// Add rankings to the leaderboard weekly ranking list.
			Lv_Leaderboard->SetListItems(Rankings);

			// Get the logged-in player's weekly rank if it is not included in the leaderboard.
			const TArray<ULeaderboardRank*> FilteredRank = Rankings.FilterByPredicate([PlayerNetId](const ULeaderboardRank* Temp) { return Temp && Temp->UserId == PlayerNetId; });
			const ULeaderboardRank* PlayerRank = FilteredRank.IsEmpty() ? nullptr : FilteredRank[0];
			if (!PlayerRank)
			{
				GetPlayerWeeklyRanking();
			}
			// Display the weekly rankings if it is not empty.
			else
			{
				DisplayPlayerWeeklyRank(PlayerRank);
				Ws_Leaderboard->SetWidgetState(
					Lv_Leaderboard->GetNumItems() <= 0 ?
					EAccelByteWarsWidgetSwitcherState::Empty :
					EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
		}
	));
}

void ULeaderboardWeeklyWidget::GetPlayerWeeklyRanking()
{
	PeriodicLeaderboardSubsystem->GetPlayerPeriodicRanking(
		GetOwningPlayer(),
		LeaderboardCode,
		CycleId,
		FOnGetLeaderboardRankingComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const TArray<ULeaderboardRank*> Rankings)
		{
			// Get and display the logged-in player's weekly rank.
			DisplayPlayerWeeklyRank((!bWasSuccessful || Rankings.IsEmpty()) ? nullptr : Rankings[0]);

			// Display the weekly rankings if it is not empty.
			Ws_Leaderboard->SetWidgetState(
				Lv_Leaderboard->GetNumItems() <= 0 ?
				EAccelByteWarsWidgetSwitcherState::Empty :
				EAccelByteWarsWidgetSwitcherState::Not_Empty);
		}
	));
}

void ULeaderboardWeeklyWidget::DisplayPlayerWeeklyRank(const ULeaderboardRank* PlayerRank)
{
	// Display player rank information.
	const bool bIsRanked = (PlayerRank && PlayerRank->Rank > 0);
	ULeaderboardRank* PlayerRankToDisplay = NewObject<ULeaderboardRank>();
	PlayerRankToDisplay->DisplayName = bIsRanked ? RANKED_MESSAGE.ToString() : UNRANKED_MESSAGE.ToString();
	PlayerRankToDisplay->Rank = bIsRanked ? PlayerRank->Rank : -1;
	PlayerRankToDisplay->Score = bIsRanked ? PlayerRank->Score : -1;

	PlayerRankPanel->SetLeaderboardRank(PlayerRankToDisplay);
	PlayerRankPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
}