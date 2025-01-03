// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/PeriodicLeaderboard/PeriodicBoardSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Core/AccelByteRegistry.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

void UPeriodicBoardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
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

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-GetUniqueNetIdFromPlayerController
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
// @@@SNIPEND

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-GetLocalUserNumFromPlayerController
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
// @@@SNIPEND

#pragma region Module.13 Function Definitions

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-GetPeriodicRankings
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
// @@@SNIPEND

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-GetPlayerPeriodicRanking
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
// @@@SNIPEND

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-GetLeaderboardCycleIdByName
void UPeriodicBoardSubsystem::GetLeaderboardCycleIdByName(const FString& InCycleName, const EAccelByteCycle& InCycleType, const FOnGetLeaderboardsCycleIdComplete& OnComplete)
{
    AccelByte::FRegistry::Statistic.GetListStatCycleConfigs(
        InCycleType,
        THandler<FAccelByteModelsStatCycleConfigPagingResult>::CreateWeakLambda(this, [InCycleName, OnComplete](const FAccelByteModelsStatCycleConfigPagingResult& Result)
            {
                FString FoundCycleId;
                for (auto& Cycle : Result.Data)
                {
                    if (Cycle.Name.Equals(InCycleName))
                    {
                        FoundCycleId = Cycle.Id;
                        break;
                    }
                }

                if (!FoundCycleId.IsEmpty())
                {
                    UE_LOG_PERIODIC_LEADERBOARD(Log, TEXT("Cycle ID of cycle with name %s is %s."), *InCycleName, *FoundCycleId);
                    OnComplete.ExecuteIfBound(true, FoundCycleId);
                }
                else
                {
                    UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cycle ID of cycle with name %s is not found."), *InCycleName);
                    OnComplete.ExecuteIfBound(false, FoundCycleId);
                }
            }),
        FErrorHandler::CreateWeakLambda(this, [InCycleName, OnComplete](int32 ErrorCode, const FString& ErrorMessage)
            {
                UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Failed to get Cycle ID of cycle with name %s. Error %d: %s"), *InCycleName, ErrorCode, *ErrorMessage);
                OnComplete.ExecuteIfBound(false, FString());
            })
        );
}
// @@@SNIPEND

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-OnGetPeriodicRankingsComplete
void UPeriodicBoardSubsystem::OnGetPeriodicRankingsComplete(
    bool bWasSuccessful,
    const int32 LocalUserNum,
    const FOnlineLeaderboardReadRef LeaderboardObj,
    const FOnGetLeaderboardRankingComplete OnComplete)
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
    if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
    {
        StartupSubsystem->QueryUserInfo(
            LocalUserNum,
            LeaderboardMembers,
            FOnQueryUsersInfoCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryUserInfoComplete, LocalUserNum, LeaderboardObj, OnComplete));
    }
    else
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Startup subsystem is invalid"));
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
    }
}
// @@@SNIPEND

// @@@SNIPSTART PeriodicBoardSubsystem.cpp-OnQueryUserInfoComplete
void UPeriodicBoardSubsystem::OnQueryUserInfoComplete(
    const FOnlineError& Error,
    const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
    const int32 LocalUserNum,
    const FOnlineLeaderboardReadRef LeaderboardObj,
    const FOnGetLeaderboardRankingComplete OnComplete)
{
    if (!ensure(UserInterface))
    {
        UE_LOG_PERIODIC_LEADERBOARD(Warning, TEXT("Cannot get periodic leaderboard. User Interface is not valid."));
        return;
    }

    if (!Error.bSucceeded)
    {
        UE_LOG_PERIODIC_LEADERBOARD(
            Warning,
            TEXT("Failed to get periodic leaderboard with code: %s. Error: %s"),
            *LeaderboardObj->LeaderboardName.ToString(), *Error.ErrorMessage.ToString());
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
        return;
    }

    UE_LOG_PERIODIC_LEADERBOARD(
        Warning,
        TEXT("Success in getting the periodic leaderboard rankings with code: %s"),
        *LeaderboardObj->LeaderboardName.ToString());

    // Return periodic leaderboard information along with its members' user info.
    TArray<ULeaderboardRank*> Rankings;
    for (const FOnlineStatsRow& Row : LeaderboardObj->Rows)
    {
        if (!Row.PlayerId.IsValid())
        {
            continue;
        }

        // Get the member's display name.
        const TSharedPtr<FOnlineUser> LeaderboardMember = UserInterface->GetUserInfo(
            LocalUserNum, Row.PlayerId->AsShared().Get());
        const FString DisplayName = !LeaderboardMember->GetDisplayName().IsEmpty() ?
            LeaderboardMember->GetDisplayName() : 
            FText::Format(DEFAULT_LEADERBOARD_DISPLAY_NAME, FText::FromString(Row.NickName.Left(5))).ToString();

        // Get the member's stat value.
        float Score = 0;
        if (Row.Columns.Contains(FName("Cycle_Point")))
        {
            // The stat key is "Cycle_Point" if it was retrieved from FOnlineLeaderboardAccelByte::ReadLeaderboardCycleAroundRank().
            Row.Columns[FName("Cycle_Point")].GetValue(Score);
        }
        else if (Row.Columns.Contains(FName("Point")))
        {
            // The stat key is "Point" if it was retrieved from FOnlineLeaderboardAccelByte::ReadLeaderboardsCycle()
            Row.Columns[FName("Point")].GetValue(Score);
        }

        // Add a new ranking object.
        ULeaderboardRank* NewRanking = NewObject<ULeaderboardRank>();
        NewRanking->Init(Row.PlayerId, Row.Rank, DisplayName, Score);
        Rankings.Add(NewRanking);
    }

    OnComplete.ExecuteIfBound(true, Rankings);
}
// @@@SNIPEND

#pragma endregion
