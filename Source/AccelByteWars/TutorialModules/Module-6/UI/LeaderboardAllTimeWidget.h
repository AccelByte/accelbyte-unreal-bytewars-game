// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-6/LeaderboardSubsystem.h"
#include "LeaderboardAllTimeWidget.generated.h"

class UAccelByteWarsWidgetList;
class ULeaderboardWidgetEntry;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULeaderboardAllTimeWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;

	/**
	 * @brief Get rankings of a leaderboard.
	 */
	void GetRankings();

	/**
	 * @brief Get logged-in player rank of a leaderboard.
	 */
	void GetPlayerRanking();

	/**
	 * @brief Display logged-in player rank of a leaderboard.
	 */
	void DisplayPlayerRank(const ULeaderboardRank* PlayerRank);

	ULeaderboardSubsystem* LeaderboardSubsystem;
	FString LeaderboardCode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ResultLimit = 10;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetList* WidgetList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	ULeaderboardWidgetEntry* PlayerRankPanel;
};
