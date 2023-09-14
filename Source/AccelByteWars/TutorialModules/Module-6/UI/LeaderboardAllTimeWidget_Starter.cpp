// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-6/UI/LeaderboardAllTimeWidget_Starter.h"
#include "TutorialModules/Module-6/UI/LeaderboardsWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

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
	Super::NativeOnActivated();

	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);

	// TODO: Call functionalities to get and display leaderboard rankings.

	// Set FTUE to open all-time leaderboard config based on selected game mode.
	if (FFTUEDialogueModel* FTUELeaderboard =
		FFTUEDialogueModel::GetMetadataById("ftue_alltime_leaderboard", AssociateTutorialModule->FTUEDialogues))
	{
		FTUELeaderboard->Button1.URLArguments[0].Argument = LeaderboardCode;
	}
}

#pragma region Module.6 Function Declarations
// TODO: Add your Module.6 function definitions here.
#pragma endregion