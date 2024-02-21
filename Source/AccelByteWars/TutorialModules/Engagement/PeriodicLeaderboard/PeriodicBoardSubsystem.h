// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"
#include "Engagement/LeaderboardEssentials/LeaderboardEssentialsModels.h"
#include "PeriodicLeaderboardLog.h"
#include "PeriodicBoardSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UPeriodicBoardSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.13 Function Declarations
public:
	/**
	 * @brief Get rankings of a periodic leaderboard.
	 * @param PC PlayerController to determine who's credential is going to be used for the API call.
	 * @param LeaderboardCode The leaderboard code (id) to be read.
	 * @param CycleId The leaderboard periodic cycle to be read.
	 * @param ResultLimit The maximum limit of leaderboard entries to be read.
	 */
	void GetPeriodicRankings(const APlayerController* PC, const FString& LeaderboardCode, const FString& CycleId, const int32 ResultLimit, const FOnGetLeaderboardRankingComplete& OnComplete = FOnGetLeaderboardRankingComplete());

	/**
	 * @brief Get particular player rank of a periodic leaderboard.
	 * @param PC PlayerController to determine which player rank should be read.
	 * @param LeaderboardCode The leaderboard code (id) to be read.
	 * @param CycleId The leaderboard periodic cycle to be read.
	 */
	void GetPlayerPeriodicRanking(const APlayerController* PC, const FString& LeaderboardCode, const FString& CycleId, const FOnGetLeaderboardRankingComplete& OnComplete = FOnGetLeaderboardRankingComplete());

	/**
	* @brief Get Leaderboards Cycle Id list
	* @param CycleType Cycle type that will be retrieved, query list : empty, "INIT", "ACTIVE", "STOPPED"
	*/
	void GetLeaderboardCycleIdByName(const FString& InCycleName, const EAccelByteCycle& InCycleType, const FOnGetLeaderboardsCycleIdComplete& OnComplete = FOnGetLeaderboardsCycleIdComplete());

protected:
	/**
	 * @brief Callback when get rankings of a periodic leaderboard is complete.
	 * @param bWasSuccessful Whether the process was successful or not.
	 * @param LocalUserNum LocalUserNum used to make API call.
	 * @param LeaderboardObj The leaderboard object that contains the ranking data.
	 */
	void OnGetPeriodicRankingsComplete(bool bWasSuccessful, const int32 LocalUserNum, const FOnlineLeaderboardReadRef LeaderboardObj, const FOnGetLeaderboardRankingComplete OnComplete);
#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineLeaderboardAccelBytePtr LeaderboardInterface;

	FDelegateHandle OnLeaderboardReadCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
};
