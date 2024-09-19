// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/PeriodicLeaderboard/UI/LeaderboardWeeklyWidget.h"
#include "Engagement/LeaderboardEssentials/UI/LeaderboardsWidget.h"
#include "Engagement/LeaderboardEssentials/UI/LeaderboardWidgetEntry.h"
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

// @@@SNIPSTART LeaderboardWeeklyWidget.cpp-NativeOnActivated
// @@@MULTISNIP SetWidgetStateExample {"selectedLines": ["9"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "6-9", "43"]}
void ULeaderboardWeeklyWidget::NativeOnActivated()
{
	// Set leaderboard code based on board-unreal-highestscore-{gamemode} format. 
	LeaderboardCode = FString::Printf(TEXT("board-unreal-highestscore-%s"), *ULeaderboardsWidget::GetLeaderboardGameMode());

	// Reset widgets.
	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	Lv_Leaderboard->ClearListItems();
	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	PeriodicLeaderboardSubsystem->GetLeaderboardCycleIdByName(FString("unreal-weekly"), EAccelByteCycle::WEEKLY,
		FOnGetLeaderboardsCycleIdComplete::CreateWeakLambda(this, [this] (bool IsSuccessful, const FString& ResultCycleId)
			{
				if (IsSuccessful && !ResultCycleId.IsEmpty())
				{
					// Set cycle id to the weekly leaderboard's cycle id.
					CycleId = ResultCycleId;

					if (FFTUEDialogueModel* FTUELeaderboard = FFTUEDialogueModel::GetMetadataById("ftue_weekly_leaderboard", FTUEDialogues))
					{
						FTUELeaderboard->Button1.URLArguments[1].Argument = LeaderboardCode;
						FTUELeaderboard->Button1.URLArguments[2].Argument = CycleId;
					}

					Super::NativeOnActivated();

					// Reset widgets.
					PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
					Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
					Lv_Leaderboard->ClearListItems();

					// Get leaderboard weekly rankings.
					GetWeeklyRankings();
				} 
				else
				{
					// Error if cycle ID is not found
					Ws_Leaderboard->ErrorMessage = FText(CYCLE_ID_NOT_FOUND_TEXT);
					Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
				}
			})
	);
}
// @@@SNIPEND

// @@@SNIPSTART LeaderboardWeeklyWidget.cpp-GetWeeklyRankings
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "45"]}
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
// @@@SNIPEND

// @@@SNIPSTART LeaderboardWeeklyWidget.cpp-GetPlayerWeeklyRanking
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "19"]}
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
// @@@SNIPEND

// @@@SNIPSTART LeaderboardWeeklyWidget.cpp-DisplayPlayerWeeklyRank
void ULeaderboardWeeklyWidget::DisplayPlayerWeeklyRank(const ULeaderboardRank* PlayerRank)
{
	// Display player rank information.
	const bool bIsRanked = (PlayerRank && PlayerRank->Rank > 0);
	ULeaderboardRank* PlayerRankToDisplay = NewObject<ULeaderboardRank>();
	PlayerRankToDisplay->Init(
		bIsRanked ? PlayerRank->UserId : nullptr,
		bIsRanked ? PlayerRank->Rank : -1,
		bIsRanked ? RANKED_MESSAGE.ToString() : UNRANKED_MESSAGE.ToString(),
		bIsRanked ? PlayerRank->Score : -1);

	PlayerRankPanel->SetLeaderboardRank(PlayerRankToDisplay);
	PlayerRankPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
}
// @@@SNIPEND
