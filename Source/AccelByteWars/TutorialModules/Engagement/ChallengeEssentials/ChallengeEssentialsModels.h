// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Models/AccelByteChallengeModels.h"
#include "ChallengeEssentialsModels.generated.h"

// @@@SNIPSTART ChallengeEssentialsModels.h-ChallengeGoalData
UCLASS()
class ACCELBYTEWARS_API UChallengeGoalData : public UObject
{
    GENERATED_BODY()

public:
    FAccelByteModelsChallengeGoal Goal;
    FAccelByteModelsChallengeGoalProgress Progress;
    TArray<FChallengeGoalRewardData> Rewards{};
    FString EndDateTime = TEXT("");

    FString GetEndTimeDuration()
    {
        FDateTime ParsedEndDateTime{};
        if (!FDateTime::ParseIso8601(*EndDateTime, ParsedEndDateTime))
        {
            return TEXT("");
        }

        /* Return duration in "dd hh mm" format.
         * If duration is less than a minute, return "< 1m" instead. */
        const FTimespan Duration = ParsedEndDateTime - FDateTime::UtcNow();
        FString DurationStr{};
        if (Duration.GetDays() > 0)
        {
            DurationStr += FString::Printf(TEXT("%dd "), Duration.GetDays());
        }
        if (Duration.GetHours() > 0)
        {
            DurationStr += FString::Printf(TEXT("%dh "), Duration.GetHours());
        }
        if (Duration.GetMinutes() > 0)
        {
            DurationStr += FString::Printf(TEXT("%dm"), Duration.GetMinutes());
        }
        else
        {
            DurationStr = TEXT("< 1m");
        }

        return DurationStr;
    }
};
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsModels.h-ChallengeGoalRewardData
USTRUCT()
struct ACCELBYTEWARS_API FChallengeGoalRewardData
{
    GENERATED_BODY()

    FString Sku = TEXT("");
    FString Name = TEXT("");
    FString IconUrl = TEXT("");
    int32 Quantity = 0;
};
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsModels.h-stringmacro
#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"
#define INVALID_CHALLENGE_INTERFACE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Invalid Challenge Interface", "Invalid Challenge Interface")
#define EMPTY_CHALLENGE_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Empty Challenge", "No Challenge Found")
#define EMPTY_CLAIMABLE_CHALLENGE_REWARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Empty Claimable Challenge Reward", "No Claimable Challenge Reward Found")
#define CLAIMED_CHALLENGE_REWARD_LABEL NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Claimed Challenge Reward Label", "Claimed")
#define CLAIMABLE_CHALLENGE_REWARD_LABEL NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Claimable Challenge Reward Label", "Claim")
#define ALLTIME_CHALLENGE_TITLE_LABEL NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "All Time Challenges", "All Time Challenges")
#define PERIODIC_CHALLENGE_TITLE_LABEL NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "{0} Challenges", "{0} Challenges")
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsModels.h-delegatemacro
DECLARE_DELEGATE_ThreeParams(FOnGetChallengeCodeComplete, bool bWasSuccessful, const FString& ErrorMessage, const FAccelByteModelsChallenge& Result);
DECLARE_DELEGATE_ThreeParams(FOnGetChallengeGoalsComplete, bool bWasSuccessful, const FString& ErrorMessage, const TArray<UChallengeGoalData*>& Result);
DECLARE_DELEGATE_TwoParams(FOnQueryRewardItemsBySkusRecursivelyComplete, bool bWasSuccessful, const FString& ErrorMessage);
DECLARE_DELEGATE_ThreeParams(FOnQueryRewardItemsInformationComplete, bool bWasSuccessful, const FString& ErrorMessage, const TArray<UChallengeGoalData*>& Result);
DECLARE_DELEGATE_TwoParams(FOnClaimChallengeGoalRewardsComplete, bool bWasSuccessful, const FString& ErrorMessage);
// @@@SNIPEND
