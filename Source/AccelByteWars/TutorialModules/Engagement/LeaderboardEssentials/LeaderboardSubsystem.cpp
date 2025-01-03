// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/LeaderboardEssentials/LeaderboardSubsystem.h"
#include "OnlineSubsystemUtils.h"

void ULeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Leaderboard Interface and make sure it's valid.
    LeaderboardInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
    if (!ensure(LeaderboardInterface.IsValid()))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }
}

// @@@SNIPSTART LeaderboardSubsystem.cpp-GetUniqueNetIdFromPlayerController
FUniqueNetIdPtr ULeaderboardSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
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

// @@@SNIPSTART LeaderboardSubsystem.cpp-GetLocalUserNumFromPlayerController
int32 ULeaderboardSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
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

#pragma region Module.6 Function Definitions

// @@@SNIPSTART LeaderboardSubsystem.cpp-GetRankings
void ULeaderboardSubsystem::GetRankings(const APlayerController* PC, const FString& LeaderboardCode, const int32 ResultLimit, const FOnGetLeaderboardRankingComplete& OnComplete)
{
    if (!ensure(LeaderboardInterface.IsValid()) || !ensure(UserInterface.IsValid()))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get leaderboard rankings. Leaderboard Interface or User Interface is not valid."));
        return;
    }

    if (!ensure(PC)) 
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get leaderboard rankings. PlayerController is null."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    FOnlineLeaderboardReadRef LeaderboardObj = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    LeaderboardObj->LeaderboardName = FName(LeaderboardCode);

    // Get the leaderboard within the range of 0 to ResultLimit.
    OnLeaderboardReadCompleteDelegateHandle = LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &ThisClass::OnGetRankingsComplete, LocalUserNum, LeaderboardObj, OnComplete));
    LeaderboardInterface->ReadLeaderboardsAroundRank(0, ResultLimit, LeaderboardObj);
}
// @@@SNIPEND

// @@@SNIPSTART LeaderboardSubsystem.cpp-GetPlayerRanking
void ULeaderboardSubsystem::GetPlayerRanking(const APlayerController* PC, const FString& LeaderboardCode, const FOnGetLeaderboardRankingComplete& OnComplete)
{
    if (!ensure(LeaderboardInterface.IsValid()) || !ensure(UserInterface.IsValid()))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get player leaderboard ranking. Leaderboard Interface or User Interface is not valid."));
        return;
    }

    if (!ensure(PC))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get player leaderboard ranking. PlayerController is null."));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetUniqueNetIdFromPlayerController(PC);
    if (!ensure(PlayerNetId.IsValid()))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get player leaderboard ranking. Player's UniqueNetId is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    FOnlineLeaderboardReadRef LeaderboardObj = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    LeaderboardObj->LeaderboardName = FName(LeaderboardCode);

    // Get the player's leaderboard ranking.
    OnLeaderboardReadCompleteDelegateHandle = LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &ThisClass::OnGetRankingsComplete, LocalUserNum, LeaderboardObj, OnComplete));
    LeaderboardInterface->ReadLeaderboards(TPartyMemberArray{ PlayerNetId->AsShared() }, LeaderboardObj);
}
// @@@SNIPEND

// @@@SNIPSTART LeaderboardSubsystem.cpp-OnGetRankingsComplete
void ULeaderboardSubsystem::OnGetRankingsComplete(bool bWasSuccessful, const int32 LocalUserNum, const FOnlineLeaderboardReadRef LeaderboardObj, const FOnGetLeaderboardRankingComplete OnComplete)
{
    ensure(UserInterface);
    ensure(LeaderboardInterface);

    LeaderboardInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadCompleteDelegateHandle);

    if (!bWasSuccessful) 
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Failed to get leaderboard rankings with code: %s"), *LeaderboardObj->LeaderboardName.ToString());
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
        return;
    }

    // Collect leaderboard members' player id.
    TPartyMemberArray LeaderboardMembers;
    for (const FOnlineStatsRow& Row : LeaderboardObj->Rows)
    {
        if (Row.PlayerId.IsValid()) 
        {
            LeaderboardMembers.Add(Row.PlayerId->AsShared());
        }
    }

    // Query leaderboard members' user information.
    if (UStartupSubsystem* StartupSubsystem = GetGameInstance()->GetSubsystem<UStartupSubsystem>())
    {
        StartupSubsystem->QueryUserInfo(
            LocalUserNum,
            LeaderboardMembers,
            FOnQueryUsersInfoCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryUserInfoComplete, LocalUserNum, LeaderboardObj, OnComplete));
    }
    else
    {
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
    }
}
// @@@SNIPEND

// @@@SNIPSTART LeaderboardSubsystem.cpp-OnQueryUserInfoComplete
void ULeaderboardSubsystem::OnQueryUserInfoComplete(
    const FOnlineError& Error,
    const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
    const int32 LocalUserNum,
    const FOnlineLeaderboardReadRef LeaderboardObj,
    const FOnGetLeaderboardRankingComplete OnComplete)
{
    if (!ensure(UserInterface))
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get leaderboard. User Interface is not valid."));
        return;
    }

    if (!Error.bSucceeded)
    {
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Failed to get leaderboard with code: %s. Error: %s"), *Error.ErrorCode, *Error.ErrorMessage.ToString());
        OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
        return;
    }

    UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Success in getting the leaderboard rankings with code: %s"), *LeaderboardObj->LeaderboardName.ToString());

    // Return leaderboard information along with its members' user info.
    TArray<ULeaderboardRank*> Rankings;
    for (const FOnlineStatsRow& Row : LeaderboardObj->Rows)
    {
        if (!Row.PlayerId.IsValid())
        {
            continue;
        }

        // Get the member's display name.
        const TSharedPtr<FOnlineUser> LeaderboardMember =
            UserInterface->GetUserInfo(LocalUserNum, Row.PlayerId->AsShared().Get());
        const FString DisplayName = !LeaderboardMember->GetDisplayName().IsEmpty() ?
            LeaderboardMember->GetDisplayName() :
            FText::Format(DEFAULT_LEADERBOARD_DISPLAY_NAME, FText::FromString(Row.NickName.Left(5))).ToString();

        // Get the member's stat value.
        float Score = 0.0f;
        if (Row.Columns.Contains(FName("AllTime_Point")))
        {
            // The stat key is "AllTime_Point" if it was retrieved from FOnlineLeaderboardAccelByte::ReadLeaderboardsAroundRank().
            Row.Columns[FName("AllTime_Point")].GetValue(Score);
        }
        else if (Row.Columns.Contains(FName("Point")))
        {
            // The stat key is "Point" if it was retrieved from FOnlineLeaderboardAccelByte::ReadLeaderboards()
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
