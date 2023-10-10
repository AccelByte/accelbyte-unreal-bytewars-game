// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-6/UI/LeaderboardAllTimeWidget_Starter.h"
#include "TutorialModules/Module-6/UI/LeaderboardsWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"

void ULeaderboardAllTimeWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	LeaderboardSubsystem = GameInstance->GetSubsystem<ULeaderboardSubsystem_Starter>();
	ensure(LeaderboardSubsystem);
}

void ULeaderboardAllTimeWidget_Starter::NativeOnActivated()
{
	// Set leaderboard code based on board-unreal-highestscore-{gamemode} format. 
	LeaderboardCode = FString::Printf(TEXT("board-unreal-highestscore-%s"), *ULeaderboardsWidget::GetLeaderboardGameMode());

	if (FFTUEDialogueModel* FTUELeaderboard = FFTUEDialogueModel::GetMetadataById("ftue_alltime_leaderboard"))
	{
		FTUELeaderboard->Button1.URLArguments[0].Argument = LeaderboardCode;
	}

	Super::NativeOnActivated();

	// Reset widgets.
	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Leaderboard->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_Leaderboard->ClearListItems();

	// TODO: Call functionalities to get and display leaderboard rankings.
}

#pragma region Module.6 Function Declarations
// TODO: Add your Module.6 function definitions here.
#pragma endregion