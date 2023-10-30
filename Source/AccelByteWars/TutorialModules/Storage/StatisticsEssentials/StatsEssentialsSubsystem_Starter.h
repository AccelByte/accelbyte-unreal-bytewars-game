// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStatsEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
#pragma region Available stats code list
	inline static FString StatsCode_HighestElimination = "unreal-highestscore-elimination";
	inline static FString StatsCode_HighestTeamDeathMatch = "unreal-highestscore-teamdeathmatch";
	inline static FString StatsCode_HighestSinglePlayer = "unreal-highestscore-singleplayer";
	inline static FString StatsCode_KillCount = "unreal-killcount";
#pragma endregion

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	UFUNCTION()
	void UpdatePlayersStatOnGameEnds();

	IOnlineIdentityPtr IdentityPtr;
	FOnlineStatisticAccelBytePtr ABStatsPtr;

	FOnlineStatsQueryUsersStatsComplete OnQueryUsersStatsComplete;
	FOnlineStatsUpdateStatsComplete OnUpdateStatsComplete;
	FOnUpdateMultipleUserStatItemsComplete OnServerUpdateStatsComplete;
};
