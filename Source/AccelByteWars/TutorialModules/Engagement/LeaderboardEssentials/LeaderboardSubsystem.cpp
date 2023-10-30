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
        UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
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


#pragma region Module.6 Function Definitions

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
    OnQueryUserInfoCompleteDelegateHandle = UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(
        LocalUserNum,
        FOnQueryUserInfoCompleteDelegate::CreateWeakLambda(this, [this, LeaderboardObj, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorStr)
        {
            if (!ensure(UserInterface)) 
            {
                UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Cannot get leaderboard. User Interface is not valid."));
                return;
            }
            UserInterface->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegateHandle);

            if (!bWasSuccessful) 
            {
                UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Failed to get leaderboard with code: %s. Error: %s"), *LeaderboardObj->LeaderboardName.ToString(), *ErrorStr);
                OnComplete.ExecuteIfBound(false, TArray<ULeaderboardRank*>());
                return;
            }

            UE_LOG_LEADERBOARD_ESSENTIALS(Warning, TEXT("Success to get leaderboard rankings with code: %s"), *LeaderboardObj->LeaderboardName.ToString());

            // Return leaderboard information along with its members' user info.
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

                // Get the member's stat value.
                float Score = 0;
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