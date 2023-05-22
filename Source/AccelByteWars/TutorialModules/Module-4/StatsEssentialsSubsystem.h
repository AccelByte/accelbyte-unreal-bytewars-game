// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStatsEssentialsSubsystem : public UGameInstanceSubsystem
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

	/**
	 * @brief Update users stats
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call
	 * @param UpdatedUsersStats User stats to be updated
	 * @param OnCompleteClient Call upon completion if current instance is not a dedicated server
	 * @param OnCompleteServer Call upon completion if current instance is a dedicated server
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool UpdateUsersStats(
		const int32 LocalUserNum,
		const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUsersStats,
		const FOnlineStatsUpdateStatsComplete& OnCompleteClient = {},
		const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer = {});

	/**
	 * @brief Query local user's stats
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call and who's stats to be retrieved
	 * @param StatNames Stats code to be retrieved. Set empty array to retrieve all of the user's stats
	 * @param OnComplete Call upon completion
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool QueryLocalUserStats(
		const int32 LocalUserNum,
		const TArray<FString>& StatNames,
		const FOnlineStatsQueryUsersStatsComplete& OnComplete);

	/**
	 * @brief Query users's stats
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call
	 * @param StatsUsers Users who's stats to be retrieved
	 * @param StatNames Stats code to be retrieved. Set empty array to retrieve all of the user's stats
	 * @param OnComplete Call upon completion
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool QueryUserStats(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& StatsUsers,
		const TArray<FString>& StatNames,
		const FOnlineStatsQueryUsersStatsComplete& OnComplete);

	/**
	 * @brief Reset all connected users stats value to 0
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call
	 * @param OnCompleteClient Call upon completion if current instance is not a dedicated server
	 * @param OnCompleteServer Call upon completion if current instance is a dedicated server
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool ResetConnectedUsersStats(
		const int32 LocalUserNum,
		const FOnlineStatsUpdateStatsComplete& OnCompleteClient = {},
		const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer = {});

private:
	UFUNCTION()
	void UpdatePlayersStatOnGameEnds();

	IOnlineIdentityPtr IdentityPtr;
	FOnlineStatisticAccelBytePtr ABStatsPtr;

	FOnlineStatsQueryUsersStatsComplete OnQueryUsersStatsComplete;
	FOnlineStatsUpdateStatsComplete OnUpdateStatsComplete;
	FOnUpdateMultipleUserStatItemsComplete OnServerUpdateStatsComplete;
};
