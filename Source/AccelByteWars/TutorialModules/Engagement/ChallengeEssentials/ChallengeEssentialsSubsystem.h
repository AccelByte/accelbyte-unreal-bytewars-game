// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"

#include "Api/AccelByteChallengeApi.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByte.h"

#include "ChallengeEssentialsModels.h"
#include "ChallengeEssentialsLog.h"
#include "ChallengeEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UChallengeEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART ChallengeEssentialsSubsystem.h-public
// @@@MULTISNIP GetChallengeByPeriod {"selectedLines": ["1-4"]}
// @@@MULTISNIP GetChallengeGoalList {"selectedLines": ["1", "5-8"]}
// @@@MULTISNIP ClaimChallengeGoalRewards {"selectedLines": ["1", "9-11"]}
public:
	void GetChallengeByPeriod(
		const EAccelByteModelsChallengeRotation Period, 
		const FOnGetChallengeCodeComplete& OnComplete = FOnGetChallengeCodeComplete());
	void GetChallengeGoalList(
		const FUniqueNetIdPtr UserId, 
		const FAccelByteModelsChallenge& Challenge,
		const FOnGetChallengeGoalsComplete& OnComplete = FOnGetChallengeGoalsComplete());
	void ClaimChallengeGoalRewards(
		const TArray<FString>& RewardIDs, 
		const FOnClaimChallengeGoalRewardsComplete& OnComplete = FOnClaimChallengeGoalRewardsComplete());
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.h-protected
// @@@MULTISNIP GetChallengeApi {"selectedLines": ["1-2"]}
// @@@MULTISNIP GetStoreInterface {"selectedLines": ["1", "3"]}
protected:
	AccelByte::Api::ChallengePtr GetChallengeApi() const;
	FOnlineStoreV2AccelBytePtr GetStoreInterface() const;
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.h-private
// @@@MULTISNIP OnGetChallengeGoalListComplete {"selectedLines": ["1-6"]}
// @@@MULTISNIP QueryRewardItemsInformation {"selectedLines": ["1", "8-11"]}
// @@@MULTISNIP QueryRewardItemsBySkusRecursively {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP OnQueryRewardItemsInformationComplete {"selectedLines": ["1", "16-20"]}
private:
	void OnGetChallengeGoalListComplete(
		bool bIsSucceeded, 
		const FString& ErrorMessage,
		const TArray<UChallengeGoalData*> Goals,
		const FOnGetChallengeGoalsComplete OnComplete);

	void QueryRewardItemsInformation(
		const FUniqueNetIdPtr UserId,
		const TArray<UChallengeGoalData*> Goals,
		const FOnQueryRewardItemsInformationComplete& OnComplete);
	void QueryRewardItemsBySkusRecursively(
		const FUniqueNetIdPtr UserId,
		TArray<FString> ItemSkusToQuery,
		const FOnQueryRewardItemsBySkusRecursivelyComplete& OnComplete);
	void OnQueryRewardItemsInformationComplete(
		bool bWasSuccessful,
		const FString& Error,
		const TArray<UChallengeGoalData*> Goals,
		const FOnQueryRewardItemsInformationComplete OnComplete);
// @@@SNIPEND
};
