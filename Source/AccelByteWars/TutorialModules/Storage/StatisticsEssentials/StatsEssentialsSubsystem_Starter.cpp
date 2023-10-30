// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsEssentialsSubsystem_Starter.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

void UStatsEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	ensure(Subsystem);

	const IOnlineStatsPtr StatsPtr = Subsystem->GetStatsInterface();
	ensure(StatsPtr);

	ABStatsPtr = StaticCastSharedPtr<FOnlineStatisticAccelByte>(StatsPtr);
	ensure(ABStatsPtr);

	IdentityPtr = Subsystem->GetIdentityInterface();
	ensure(IdentityPtr);

	// bind delegate if module active
	if (UTutorialModuleUtility::IsTutorialModuleActive(FPrimaryAssetId{ "TutorialModule:STATSESSENTIALS" }, this))
	{
		AAccelByteWarsInGameGameMode::OnGameEndsDelegate.AddUObject(this, &ThisClass::UpdatePlayersStatOnGameEnds);
	}
}

void UStatsEssentialsSubsystem_Starter::UpdatePlayersStatOnGameEnds()
{
}
