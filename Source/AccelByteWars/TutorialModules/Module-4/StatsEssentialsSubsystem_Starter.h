// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "StatsEssentialsSubsystem_Starter.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UStatsEssentialsSubsystem_Starter : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
#pragma region Available stats code list
	inline static FString StatsCode_HighestElimination = "ue-highestscore-elimintaion";
	inline static FString StatsCode_HighestTeamDeathMatch = "ue-highestscore-teamdeathmatch";
	inline static FString StatsCode_HighestSinglePlayer = "ue-highestscore-singleplayer";
	inline static FString StatsCode_KillCount = "ue-killcount";
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
