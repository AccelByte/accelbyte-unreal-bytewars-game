// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/UI/LeaderboardWeeklyWidget_Starter.h"
#include "TutorialModules/Module-6/UI/LeaderboardsWidget.h"
#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

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
	Super::NativeOnActivated();

	PlayerRankPanel->SetVisibility(ESlateVisibility::Collapsed);
	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);

	// TODO: Call functionalities to get and display leaderboard weekly rankings.
	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

#pragma region Module.13 Function Definitions
// TODO: Add your Module.13 function definitions here.
#pragma endregion
