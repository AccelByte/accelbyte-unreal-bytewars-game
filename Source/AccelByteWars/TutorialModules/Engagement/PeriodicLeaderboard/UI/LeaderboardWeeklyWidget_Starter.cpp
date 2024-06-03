// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/PeriodicLeaderboard/UI/LeaderboardWeeklyWidget_Starter.h"
#include "Engagement/LeaderboardEssentials/UI/LeaderboardsWidget.h"
#include "Engagement/LeaderboardEssentials/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"

void ULeaderboardWeeklyWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PeriodicLeaderboardSubsystem = GameInstance->GetSubsystem<UPeriodicBoardSubsystem_Starter>();
	ensure(PeriodicLeaderboardSubsystem);
}

void ULeaderboardWeeklyWidget_Starter::NativeOnActivated()
{
	// Set leaderboard code based on board-unreal-highestscore-{gamemode} format. 
	LeaderboardCode = FString::Printf(TEXT("board-unreal-highestscore-%s"), *ULeaderboardsWidget::GetLeaderboardGameMode());

	// Set cycle id to the weekly leaderboard's cycle id.
	CycleId = FString("<REPLACE_WITH_LEADERBOARD_ID>");

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

	// TODO: Call functionalities to get and display leaderboard weekly rankings.
}

#pragma region Module.13 Function Definitions
// TODO: Add your Module.13 function definitions here.
#pragma endregion
