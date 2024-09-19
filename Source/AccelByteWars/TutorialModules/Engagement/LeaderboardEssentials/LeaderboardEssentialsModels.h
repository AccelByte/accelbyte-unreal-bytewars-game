// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "LeaderboardEssentialsModels.generated.h"

// @@@SNIPSTART LeaderboardEssentialsModels.h-LeaderboardRankClass
UCLASS()
class ACCELBYTEWARS_API ULeaderboardRank : public UObject
{
    GENERATED_BODY()

public:
    FUniqueNetIdRepl UserId;
    int32 Rank;
    FString DisplayName;
    float Score;

    void Init(const FUniqueNetIdRepl InUserId, const int32 InRank, const FString InDisplayName, const float InScore)
    {
        UserId = InUserId;
        Rank = InRank;
        DisplayName = InDisplayName;
        Score = InScore;
    }
};
// @@@SNIPEND

#define DEFAULT_LEADERBOARD_DISPLAY_NAME NSLOCTEXT("AccelByteWars", "Player-{0}", "Player-{0}")
#define RANKED_MESSAGE NSLOCTEXT("AccelByteWars", "Your Rank", "Your Rank")
#define UNRANKED_MESSAGE NSLOCTEXT("AccelByteWars", "You Are Unranked", "You Are Unranked")
#define CYCLE_ID_NOT_FOUND_TEXT NSLOCTEXT("AccelByteWars", "Can't Find Cycle ID for this cycle", "Can't Find Cycle ID for this cycle")

// @@@SNIPSTART LeaderboardEssentialsModels.h-delegatemacro
// @@@MULTISNIP GetLeaderboardRankingDelegate {"selectedLines": ["1"]}
// @@@MULTISNIP GetLeaderboardCycleIdDelegate {"selectedLines": ["2"]}
DECLARE_DELEGATE_TwoParams(FOnGetLeaderboardRankingComplete, bool /*bWasSuccessful*/, const TArray<ULeaderboardRank*> /*Rankings*/);
DECLARE_DELEGATE_TwoParams(FOnGetLeaderboardsCycleIdComplete, bool /*bWasSuccesful*/, const FString& CycleId);
// @@@SNIPEND
