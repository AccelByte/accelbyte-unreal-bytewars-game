// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/PeriodicBoardSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UPeriodicBoardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Leaderboard Interface and make sure it's valid.
    LeaderboardInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
    if (!ensure(LeaderboardInterface.IsValid()))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Leaderboard Interface is not valid."));
        return;
    }
}

FUniqueNetIdPtr UPeriodicBoardSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
{
    if (!ensure(PC))
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!ensure(LocalPlayer))
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

int32 UPeriodicBoardSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
{
    if (!PC)
    {
        return INDEX_NONE;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return INDEX_NONE;
    }

    return LocalPlayer->GetControllerId();
}


#pragma region Module.13 Function Definitions

void UPeriodicBoardSubsystem::GetPeriodicRankings(const APlayerController* PC, const FString& LeaderboardCode, const FString& CycleId, const int32 ResultLimit, const FOnGetLeaderboardRankingComplete& OnComplete)
{
    if (!ensure(LeaderboardInterface.IsValid()) || !ensure(UserInterface.IsValid()))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get periodic leaderboard rankings. Leaderboard Interface or User Interface is not valid."));
        return;
    }

    if (!ensure(PC))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get periodic leaderboard rankings. PlayerController is null."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    FOnlineLeaderboardReadRef LeaderboardObj = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    LeaderboardObj->LeaderboardName = FName(LeaderboardCode);

    // Get the periodic leaderboard within the range of 0 to ResultLimit.
    OnLeaderboardReadCompleteDelegateHandle = LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &ThisClass::OnGetPeriodicRankingsComplete, LocalUserNum, LeaderboardObj, OnComplete));
    LeaderboardInterface->ReadLeaderboardCycleAroundRank(0, ResultLimit, CycleId, LeaderboardObj);
}

void UPeriodicBoardSubsystem::GetPlayerPeriodicRanking(const APlayerController* PC, const FString& LeaderboardCode, const FString& CycleId, const FOnGetLeaderboardRankingComplete& OnComplete)
{
    if (!ensure(LeaderboardInterface.IsValid()) || !ensure(UserInterface.IsValid()))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get player periodic leaderboard ranking. Leaderboard Interface or User Interface is not valid."));
        return;
    }

    if (!ensure(PC))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get player periodic leaderboard ranking. PlayerController is null."));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetUniqueNetIdFromPlayerController(PC);
    if (!ensure(PlayerNetId.IsValid()))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get player periodic leaderboard ranking. Player's UniqueNetId is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    FOnlineLeaderboardReadRef LeaderboardObj = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    LeaderboardObj->LeaderboardName = FName(LeaderboardCode);

    // Get the player's periodic leaderboard ranking.
    OnLeaderboardReadCompleteDelegateHandle = LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &ThisClass::OnGetPeriodicRankingsComplete, LocalUserNum, LeaderboardObj, OnComplete));
    LeaderboardInterface->ReadLeaderboardsCycle(TPartyMemberArray{ PlayerNetId->AsShared() }, LeaderboardObj, CycleId);
}

void UPeriodicBoardSubsystem::OnGetPeriodicRankingsComplete(bool bWasSuccessful, const int32 LocalUserNum, const FOnlineLeaderboardReadRef LeaderboardObj, const FOnGetLeaderboardRankingComplete OnComplete)
{
    ensure(UserInterface);
    ensure(LeaderboardInterface);

    LeaderboardInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadCompleteDelegateHandle);

    if (!bWasSuccessful)
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Failed to get periodic leaderboard rankings with code: %s"), *LeaderboardObj->LeaderboardName.ToString());
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
        return;
    }

    // Collect periodic leaderboard members' player id.
    TPartyMemberArray LeaderboardMembers;
    for (const FOnlineStatsRow& Row : LeaderboardObj->Rows)
    {
        if (Row.PlayerId.IsValid())
        {
            LeaderboardMembers.Add(Row.PlayerId->AsShared());
        }
    }

    // Query periodic leaderboard members' user information.
    OnQueryUserInfoCompleteDelegateHandle = UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(
        LocalUserNum,
        FOnQueryUserInfoCompleteDelegate::CreateWeakLambda(this, [this, LeaderboardObj, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorStr)
        {
            if (!ensure(UserInterface))
            {
                UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get periodic leaderboard. User Interface is not valid."));
                return;
            }
            UserInterface->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegateHandle);

            if (!bWasSuccessful)
            {
                UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Failed to get periodic leaderboard with code: %s. Error: %s"), *LeaderboardObj->LeaderboardName.ToString(), *ErrorStr);
                OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
                return;
            }

            UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Success to get periodic leaderboard rankings with code: %s"), *LeaderboardObj->LeaderboardName.ToString());

            // Return periodic leaderboard information along with its members' user info.
            TArray<ULeaderboardRank*> Rankings;
            for (const FOnlineStatsRow& Row : LeaderboardObj->Rows)
            {
                if (!Row.PlayerId.IsValid())
                {
                    continue;
                }

                // Get the member's display name.
                const TSharedPtr<FOnlineUser> LeaderboardMember = UserInterface->GetUserInfo(LocalUserNum, Row.PlayerId->AsShared().Get());
                const FString DisplayName = !LeaderboardMember->GetDisplayName().IsEmpty() ?
                    LeaderboardMember->GetDisplayName() :
                    FText::Format(DEFAULT_LEADERBOARD_DISPLAY_NAME, FText::FromString(Row.NickName.Left(5))).ToString();

                // Get the member's stat value. The stat key is always "Point".
                float Score;
                Row.Columns[FName("Point")].GetValue(Score);

                // Add a new ranking object.
                ULeaderboardRank* NewRanking = NewObject<ULeaderboardRank>();
                NewRanking->UserId = Row.PlayerId;
                NewRanking->Rank = Row.Rank;
                NewRanking->DisplayName = DisplayName;
                NewRanking->Score = Score;
                Rankings.Add(NewRanking);
            }

            OnComplete.ExecuteIfBound(true, Rankings);
        }
    ));

    UserInterface->QueryUserInfo(LocalUserNum, LeaderboardMembers);
}

#pragma endregion
