// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"
#include "TutorialModules/Module-6/LeaderboardEssentialsLog.h"
#include "TutorialModules/Module-6/LeaderboardEssentialsModels.h"
#include "LeaderboardSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API ULeaderboardSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#pragma region Module.6 Function Declarations
public:
	/**
	 * @brief Get rankings of a leaderboard.
	 * @param PC PlayerController to determine who's credential is going to be used for the API call.
	 * @param LeaderboardCode The leaderboard code (id) to be read.
	 * @param ResultLimit The maximum limit of leaderboard entries to be read.
	 */
	void GetRankings(const APlayerController* PC, const FString& LeaderboardCode, const int32 ResultLimit, const FOnGetLeaderboardRankingComplete& OnComplete = FOnGetLeaderboardRankingComplete());

	/**
	 * @brief Get particular player rank of a leaderboard.
	 * @param PC PlayerController to determine which player rank should be read.
	 * @param LeaderboardCode The leaderboard code (id) to be read.
	 */
	void GetPlayerRanking(const APlayerController* PC, const FString& LeaderboardCode, const FOnGetLeaderboardRankingComplete& OnComplete = FOnGetLeaderboardRankingComplete());

	/**
	 * @brief Callback when get rankings of a leaderboard is complete.
	 * @param bWasSuccessful Whether the process was successful or not.
	 * @param LocalUserNum LocalUserNum used to make API call.
	 * @param LeaderboardObj The leaderboard object that contains the ranking data.
	 */
	void OnGetRankingsComplete(bool bWasSuccessful, const int32 LocalUserNum, const FOnlineLeaderboardReadRef LeaderboardObj, const FOnGetLeaderboardRankingComplete OnComplete);

#pragma endregion

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineLeaderboardAccelBytePtr LeaderboardInterface;

	FDelegateHandle OnLeaderboardReadCompleteDelegateHandle;
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
};
