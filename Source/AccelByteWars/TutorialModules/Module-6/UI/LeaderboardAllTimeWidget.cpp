// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-6/UI/LeaderboardAllTimeWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardsWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"

void ULeaderboardAllTimeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	LeaderboardSubsystem = GameInstance->GetSubsystem<ULeaderboardSubsystem>();
	ensure(LeaderboardSubsystem);
}

void ULeaderboardAllTimeWidget::NativeOnActivated()
{
	// Set leaderboard code based on board-unreal-highestscore-{gamemode} format. 
	LeaderboardCode = FString::Printf(TEXT("board-unreal-highestscore-%s"), *ULeaderboardsWidget::GetLeaderboardGameMode());

	// Set FTUE to open all-time leaderboard config based on selected game mode.
	if (FFTUEDialogueModel* FTUELeaderboard =
		FTUEDialogueGroup::GetMetadataById("ftue_alltime_leaderboard", AssociateTutorialModule->FTUEDialogueGroups))
	{
		FTUELeaderboard->Button1.URLArguments[0].Argument = LeaderboardCode;
	}

	// Reset widgets.
	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_Leaderboard->ClearListItems();

	Super::NativeOnActivated();

	// Get leaderboard rankings.
	GetRankings();
}

void ULeaderboardAllTimeWidget::GetRankings()
{
	FUniqueNetIdRepl PlayerNetId = GetOwningPlayer()->GetLocalPlayer()->GetPreferredUniqueNetId();
	if (!PlayerNetId.IsValid()) 
	{
		return;
	}

	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	LeaderboardSubsystem->GetRankings(
		GetOwningPlayer(), 
		LeaderboardCode,
		ResultLimit,
		FOnGetLeaderboardRankingComplete::CreateWeakLambda(this, [this, PlayerNetId](bool bWasSuccessful, const TArray<ULeaderboardRank*> Rankings)
		{
			if (!bWasSuccessful) 
			{
				Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
				return;
			}

			// Add rankings to the leaderboard ranking list.
			Lv_Leaderboard->SetListItems(Rankings);

			// Get the logged-in player's rank if it is not included in the leaderboard.
			const TArray<ULeaderboardRank*> FilteredRank = Rankings.FilterByPredicate([PlayerNetId](const ULeaderboardRank* Temp) { return Temp && Temp->UserId == PlayerNetId; });
			const ULeaderboardRank* PlayerRank = FilteredRank.IsEmpty() ? nullptr : FilteredRank[0];
			if (!PlayerRank)
			{ 
				GetPlayerRanking();
			}
			// Display the rankings if it is not empty.
			else 
			{
				DisplayPlayerRank(PlayerRank);
				Ws_Leaderboard->SetWidgetState(
					Lv_Leaderboard->GetNumItems() <= 0 ?
					EAccelByteWarsWidgetSwitcherState::Empty :
					EAccelByteWarsWidgetSwitcherState::Not_Empty
				);
			}
		}
	));
}

void ULeaderboardAllTimeWidget::GetPlayerRanking()
{
	LeaderboardSubsystem->GetPlayerRanking(
		GetOwningPlayer(),
		LeaderboardCode,
		FOnGetLeaderboardRankingComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const TArray<ULeaderboardRank*> Rankings)
		{
			// Get and display the logged-in player's rank.
			DisplayPlayerRank((!bWasSuccessful || Rankings.IsEmpty()) ? nullptr : Rankings[0]);

			// Display the rankings if it is not empty.
			Ws_Leaderboard->SetWidgetState(
				Lv_Leaderboard->GetNumItems() <= 0 ?
				EAccelByteWarsWidgetSwitcherState::Empty :
				EAccelByteWarsWidgetSwitcherState::Not_Empty
			);
		}
	));
}

void ULeaderboardAllTimeWidget::DisplayPlayerRank(const ULeaderboardRank* PlayerRank)
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