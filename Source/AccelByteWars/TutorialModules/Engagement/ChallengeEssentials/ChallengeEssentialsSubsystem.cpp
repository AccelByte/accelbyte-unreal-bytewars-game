// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeEssentialsSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "OnlineSubsystemUtils.h"

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-GetChallengeByPeriod
void UChallengeEssentialsSubsystem::GetChallengeByPeriod(
    const EAccelByteModelsChallengeRotation Period, 
    const FOnGetChallengeCodeComplete& OnComplete)
{
    const AccelByte::Api::ChallengePtr ChallengeApi = GetChallengeApi();
    if (!ChallengeApi)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge. Challenge API Client is invalid."));
        OnComplete.ExecuteIfBound(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString(), FAccelByteModelsChallenge{});
        return;
    }

    // Get the list of challenges and return the one that matches with the requested period.
    ChallengeApi->GetChallenges(
        AccelByte::THandler<FAccelByteModelsGetChallengesResponse>::CreateWeakLambda(this, [Period, OnComplete](const FAccelByteModelsGetChallengesResponse& Result)
        {
            for (const FAccelByteModelsChallenge& Challenge : Result.Data)
            {
                // Skip inactive challenge.
                if (Challenge.Status == EAccelByteModelsChallengeStatus::RETIRED)
                {
                    continue;
                }

                // Challenge codes in Byte Wars use the <engine>-<period> format, e.g., unreal-alltime or unreal-weekly.
                if (Challenge.Code.Contains("unreal") && Challenge.Rotation == Period)
                {
                    UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to get challenge with %s period. Challenge code: %s"), *FAccelByteUtilities::GetUEnumValueAsString(Period), *Challenge.Code);
                    OnComplete.ExecuteIfBound(true, TEXT(""), Challenge);
                    return;
                }
            }

            UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge. No challenge found with %s period."), *FAccelByteUtilities::GetUEnumValueAsString(Period));
            OnComplete.ExecuteIfBound(false, EMPTY_CHALLENGE_MESSAGE.ToString(), FAccelByteModelsChallenge{});
        }),
        AccelByte::FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
        {
            UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge. Error %d: %s"), ErrorCode, *ErrorMessage);
            OnComplete.ExecuteIfBound(false, ErrorMessage, FAccelByteModelsChallenge{});
        }),
        EAccelByteModelsChallengeSortBy::UPDATED_AT_DESC,
        EAccelByteModelsChallengeStatus::NONE);
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-GetChallengeGoalList
void UChallengeEssentialsSubsystem::GetChallengeGoalList(
    const FUniqueNetIdPtr UserId, 
    const FAccelByteModelsChallenge& Challenge,
    const FOnGetChallengeGoalsComplete& OnComplete)
{
    if (!UserId)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. User ID is invalid."));
        OnGetChallengeGoalListComplete(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString(), {}, OnComplete);
        return;
    }

    const AccelByte::Api::ChallengePtr ChallengeApi = GetChallengeApi();
    if (!ChallengeApi)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. Challenge API Client is invalid."));
        OnGetChallengeGoalListComplete(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString(), {}, OnComplete);
        return;
    }

    // Request to evaluate to update challenge goals progresses.
    ChallengeApi->EvaluateChallengeProgress(AccelByte::FVoidHandler::CreateWeakLambda(this, [this, ChallengeApi, Challenge, UserId, OnComplete]()
    {
        // Get the goal list and their progress.
        ChallengeApi->GetChallengeProgress(
            Challenge.Code,
            AccelByte::THandler<FAccelByteModelsChallengeProgressResponse>::CreateWeakLambda(this, [this, Challenge, UserId, OnComplete](const FAccelByteModelsChallengeProgressResponse& Result)
            {
                // Construct new goal object and add it to the list.
                TArray<UChallengeGoalData*> GoalDataList{};
                for (const FAccelByteModelsChallengeGoalProgress& Progress : Result.Data)
                {
                    if (UChallengeGoalData* GoalData = NewObject<UChallengeGoalData>()) 
                    {
                        GoalData->Goal = Progress.Goal;
                        GoalData->Progress = Progress;
                        GoalData->EndDateTime = (Challenge.Rotation == EAccelByteModelsChallengeRotation::NONE) ? TEXT("") : Result.Meta.Period.EndTime.ToIso8601();
                        GoalDataList.Add(GoalData);
                    }
                }

                // Query reward item information for all goals.
                QueryRewardItemsInformation(
                    UserId, 
                    GoalDataList, 
                    FOnQueryRewardItemsInformationComplete::CreateWeakLambda(this, [this, OnComplete](bool bWasSuccessful, const FString& ErrorMessage, const TArray<UChallengeGoalData*>& Result)
                    {
                        // Operation is complete, return the result.
                        OnGetChallengeGoalListComplete(bWasSuccessful, ErrorMessage, Result, OnComplete);
                    }));
            }),
            AccelByte::FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32 ErrorCode, const FString& ErrorMessage)
            {
                OnGetChallengeGoalListComplete(false, ErrorMessage, {}, OnComplete);
            }));
    }),
    AccelByte::FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32 ErrorCode, const FString& ErrorMessage)
    {
        OnGetChallengeGoalListComplete(false, ErrorMessage, {}, OnComplete);
    }));
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-OnGetChallengeGoalListComplete
void UChallengeEssentialsSubsystem::OnGetChallengeGoalListComplete(
    bool bIsSucceeded, 
    const FString& ErrorMessage, 
    const TArray<UChallengeGoalData*> Goals,
    const FOnGetChallengeGoalsComplete OnComplete)
{
    if (!bIsSucceeded) 
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. Error: %s"), *ErrorMessage);
        OnComplete.ExecuteIfBound(false, ErrorMessage, {});
        return;   
    }

    // Sort the result based on the unclaimed rewards and completion status.
    TArray<UChallengeGoalData*> Result = Goals;
    Result.Sort([](UChallengeGoalData& Goal1, UChallengeGoalData& Goal2)
    {
        const EAccelByteModelsChallengeGoalProgressStatus CompleteStatus = EAccelByteModelsChallengeGoalProgressStatus::COMPLETED;
        const bool bIsCompleted = (Goal1.Progress.Status == CompleteStatus) > (Goal2.Progress.Status == CompleteStatus);
        const bool bIsClaimed = Goal1.Progress.ToClaimRewards.IsEmpty() < Goal2.Progress.ToClaimRewards.IsEmpty();
        return bIsClaimed || bIsCompleted;
    });

    UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to get challenge goal list."));
    OnComplete.ExecuteIfBound(true, ErrorMessage, Result);
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-QueryRewardItemsInformation
void UChallengeEssentialsSubsystem::QueryRewardItemsInformation(
    const FUniqueNetIdPtr UserId,
    const TArray<UChallengeGoalData*> Goals,
    const FOnQueryRewardItemsInformationComplete& OnComplete)
{
    if (!UserId)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items info. User ID is invalid."));
        OnQueryRewardItemsInformationComplete(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString(), Goals, OnComplete);
        return;
    }

    const FOnlineStoreV2AccelBytePtr StoreInterface = GetStoreInterface();
    if (!StoreInterface) 
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items info. Store interface is invalid."));
        OnQueryRewardItemsInformationComplete(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString(), Goals, OnComplete);
        return;
    }

    // Collect reward item SKUs to query.
    TArray<FString> RewardItemSkusToQuery{};
    for (const UChallengeGoalData* GoalData : Goals)
    {
        if (!GoalData) continue;
        for (const FAccelByteModelsChallengeGoalReward& Reward : GoalData->Goal.Rewards)
        {
            // The reward item ID from backend response is actually the item's SKU.
            RewardItemSkusToQuery.AddUnique(Reward.ItemId);
        }
    };

    // Check if the reward items information is already cached.
    TArray<FString> CachedOfferIds{};
    TArray<FOnlineStoreOfferAccelByteRef> CachedOffers{};
    StoreInterface->GetOffers(CachedOffers);
    Algo::Transform(CachedOffers, CachedOfferIds, [](const FOnlineStoreOfferAccelByteRef Item) { return Item->Sku; });
    RewardItemSkusToQuery.RemoveAll([&CachedOfferIds](const FString& Item) { return CachedOfferIds.Contains(Item); });

    // Return success if all reward items are already cached.
    if (RewardItemSkusToQuery.IsEmpty())
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to query reward items info. All infos are already cached."));
        OnQueryRewardItemsInformationComplete(true, TEXT(""), Goals, OnComplete);
        return;
    }

    // Query reward items information by SKUs recursively.
    QueryRewardItemsBySkusRecursively(
        UserId,
        RewardItemSkusToQuery,
        FOnQueryRewardItemsBySkusRecursivelyComplete::CreateUObject(this, &ThisClass::OnQueryRewardItemsInformationComplete, Goals, OnComplete));
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-QueryRewardItemsBySkusRecursively
void UChallengeEssentialsSubsystem::QueryRewardItemsBySkusRecursively(
    const FUniqueNetIdPtr UserId, 
    TArray<FString> ItemSkusToQuery,
    const FOnQueryRewardItemsBySkusRecursivelyComplete& OnComplete)
{
    // All item is queried, the operation is completed.
    if (ItemSkusToQuery.IsEmpty())
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to query reward items info by SKUs."));
        OnComplete.ExecuteIfBound(true, TEXT(""));
        return;
    }

    if (!UserId)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items info by SKUs. User ID is invalid."));
        OnComplete.ExecuteIfBound(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString());
        return;
    }

    const FOnlineStoreV2AccelBytePtr StoreInterface = GetStoreInterface();
    if (!StoreInterface)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items info by SKUs. Store interface is invalid."));
        OnComplete.ExecuteIfBound(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString());
        return;
    }

    const FString CurrentSku = ItemSkusToQuery[0];
    ItemSkusToQuery.RemoveAt(0);

    StoreInterface->QueryOfferBySku(
        UserId.ToSharedRef().Get(),
        CurrentSku,
        FOnQueryOnlineStoreOffersComplete::CreateWeakLambda(this, [this, UserId, ItemSkusToQuery, OnComplete]
        (bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
        {
            // Abort if failed to query item by SKU.
            if (!bWasSuccessful)
            {
                UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items by SKUs. Error: %s"), *Error);
                OnComplete.ExecuteIfBound(false, Error);
                return;
            }

            // Recurse to next item SKU
            QueryRewardItemsBySkusRecursively(UserId, ItemSkusToQuery, OnComplete);
        }));
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-OnQueryRewardItemsInformationComplete
void UChallengeEssentialsSubsystem::OnQueryRewardItemsInformationComplete(
    bool bWasSuccessful,
    const FString& Error,
    const TArray<UChallengeGoalData*> Goals,
    const FOnQueryRewardItemsInformationComplete OnComplete)
{
    if (!bWasSuccessful)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to query reward items info. Error: %s"), *Error);
        OnComplete.ExecuteIfBound(false, Error, {});
        return;
    }

    // Construct goal reward data based on queried information.
    for (UChallengeGoalData* GoalData : Goals)
    {
        if (!GoalData) continue;
        for (const FAccelByteModelsChallengeGoalReward& Reward : GoalData->Goal.Rewards)
        {
            // Get item offer from cache by using the item SKU (the reward item ID is the item's SKU).
            TSharedPtr<FOnlineStoreOfferAccelByte> Offer = GetStoreInterface()->GetOfferBySkuAccelByte(Reward.ItemId);

            FChallengeGoalRewardData RewardData;
            RewardData.Sku = Reward.ItemId;
            RewardData.Name = Reward.ItemName;
            RewardData.Quantity = (int32)Reward.Qty;

            FString IconKey = TEXT("IconUrl");
            if (Offer && Offer->DynamicFields.Contains(IconKey))
            {
                RewardData.IconUrl = *Offer->DynamicFields.Find(IconKey);
            }

            GoalData->Rewards.Add(RewardData);
        }
    }

    UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to query reward items info."));
    OnComplete.ExecuteIfBound(true, TEXT(""), Goals);
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-ClaimChallengeGoalRewards
void UChallengeEssentialsSubsystem::ClaimChallengeGoalRewards(
    const TArray<FString>& RewardIDs,
    const FOnClaimChallengeGoalRewardsComplete& OnComplete)
{
    const AccelByte::Api::ChallengePtr ChallengeApi = GetChallengeApi();
    if (!ChallengeApi)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to claim challenge goal reward. Challenge API Client is invalid."));
        OnComplete.ExecuteIfBound(false, INVALID_CHALLENGE_INTERFACE_MESSAGE.ToString());
        return;
    }

    ChallengeApi->ClaimReward(
        FAccelByteModelsChallengeRewardClaimRequest{ RewardIDs },
        AccelByte::THandler<TArray<FAccelByteModelsChallengeReward>>::CreateWeakLambda(this, [OnComplete](const TArray<FAccelByteModelsChallengeReward>& Result)
        {
            // Abort if there is no claimable rewards.
            if (Result.IsEmpty())
            {
                UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to claim challenge rewards. No claimable reward found."));
                OnComplete.ExecuteIfBound(false, EMPTY_CLAIMABLE_CHALLENGE_REWARD_MESSAGE.ToString());
                return;
            }

            UE_LOG_CHALLENGE_ESSENTIALS(Log, TEXT("Success to claim challenge rewards."));
            OnComplete.ExecuteIfBound(true, TEXT(""));
        }),
        AccelByte::FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
        {
            UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to claim challenge rewards. Error %d: %s"), ErrorCode, *ErrorMessage);
            OnComplete.ExecuteIfBound(false, ErrorMessage);
        }));
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-GetChallengeApi
AccelByte::Api::ChallengePtr UChallengeEssentialsSubsystem::GetChallengeApi() const
{
    AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
    if (!ApiClient)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("AccelByte API Client is invalid."));
        return nullptr;
    }

    return ApiClient->GetChallengeApi().Pin();
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeEssentialsSubsystem.cpp-GetStoreInterface
FOnlineStoreV2AccelBytePtr UChallengeEssentialsSubsystem::GetStoreInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
}
// @@@SNIPEND
