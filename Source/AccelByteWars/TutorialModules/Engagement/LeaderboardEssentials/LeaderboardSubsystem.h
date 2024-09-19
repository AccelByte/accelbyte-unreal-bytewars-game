// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"
#include "LeaderboardEssentialsLog.h"
#include "LeaderboardEssentialsModels.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "LeaderboardSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API ULeaderboardSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART LeaderboardSubsystem.h-public
// @@@MULTISNIP GetRankings {"selectedLines": ["1", "11"]}
// @@@MULTISNIP GetPlayerRanking {"selectedLines": ["1", "18"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Module.6 Function Declarations
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
// @@@SNIPEND
	
// @@@SNIPSTART LeaderboardSubsystem.h-protected
// @@@MULTISNIP Interface {"selectedLines": ["1", "21-22"]}
// @@@MULTISNIP OnGetRankingsComplete {"selectedLines": ["1", "8"]}
// @@@MULTISNIP OnQueryUserInfoComplete {"selectedLines": ["1", "10-15"]}
protected:
	/**
	 * @brief Callback when get rankings of a leaderboard is complete.
	 * @param bWasSuccessful Whether the process was successful or not.
	 * @param LocalUserNum LocalUserNum used to make API call.
	 * @param LeaderboardObj The leaderboard object that contains the ranking data.
	 */
	void OnGetRankingsComplete(bool bWasSuccessful, const int32 LocalUserNum, const FOnlineLeaderboardReadRef LeaderboardObj, const FOnGetLeaderboardRankingComplete OnComplete);

	void OnQueryUserInfoComplete(
		const FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
		const int32 LocalUserNum,
		const FOnlineLeaderboardReadRef LeaderboardObj,
		const FOnGetLeaderboardRankingComplete OnComplete);
#pragma endregion

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PC) const;
	int32 GetLocalUserNumFromPlayerController(const APlayerController* PC) const;

	FOnlineUserAccelBytePtr UserInterface;
	FOnlineLeaderboardAccelBytePtr LeaderboardInterface;

	FDelegateHandle OnLeaderboardReadCompleteDelegateHandle;
// @@@SNIPEND
};
