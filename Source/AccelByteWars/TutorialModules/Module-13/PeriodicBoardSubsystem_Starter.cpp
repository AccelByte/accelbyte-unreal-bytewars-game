// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/PeriodicBoardSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"

void UPeriodicBoardSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
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

FUniqueNetIdPtr UPeriodicBoardSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
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

int32 UPeriodicBoardSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
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
// TODO: Add your Module.13 function definitions here.
#pragma endregion