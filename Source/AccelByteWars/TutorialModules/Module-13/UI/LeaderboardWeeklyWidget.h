// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-13/PeriodicBoardSubsystem.h"
#include "LeaderboardWeeklyWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class ULeaderboardWidgetEntry;
class UListView;

UCLASS()
class ACCELBYTEWARS_API ULeaderboardWeeklyWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;

	/**
	 * @brief Get weekly rankings of a leaderboard.
	 */
	void GetWeeklyRankings();

	/**
	 * @brief Get logged-in player weekly rank of a leaderboard.
	 */
	void GetPlayerWeeklyRanking();

	/**
	 * @brief Display logged-in player weekly rank of a leaderboard.
	 */
	void DisplayPlayerWeeklyRank(const ULeaderboardRank* PlayerRank);

	UPeriodicBoardSubsystem* PeriodicLeaderboardSubsystem;
	FString LeaderboardCode;
	FString CycleId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ResultLimit = 10;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Leaderboard;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Leaderboard;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	ULeaderboardWidgetEntry* PlayerRankPanel;
};
