// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/LeaderboardEssentials/LeaderboardSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

void ULeaderboardSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
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

FUniqueNetIdPtr ULeaderboardSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
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

int32 ULeaderboardSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
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
// TODO: Add your Module.6 function definitions here.
#pragma endregion