// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "StatsEssentialsLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStatsEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART StatsEssentialsSubsystem.h-public
// @@@MULTISNIP QueryStatsDeclaration {"selectedLines": ["1", "25-28", "38-42"]}
// @@@MULTISNIP UpdateStatsDeclaration {"selectedLines": ["1", "12-16", "52-56"]}
public:
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
	 * @brief Query local users' stats
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call and who's stats to be retrieved
	 * @param StatNames Stats code to be retrieved. Set empty array to retrieve all of the users' stats
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
	 * @param StatNames Stats code to be retrieved. Set empty array to retrieve all of the users' stats
	 * @param OnComplete Call upon completion
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool QueryUserStats(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& StatsUsers,
		const TArray<FString>& StatNames,
		const FOnlineStatsQueryUsersStatsComplete& OnComplete);

	/**
	 * @brief Update all connected player statistics
	 * @param LocalUserNum LocalUserNum to determine who's credential is going to be used for the API call
	 * @param bToReset Whether to update the statistics to the new value or to reset back to 0.
	 * @param OnCompleteClient Call upon completion if current instance is not a dedicated server
	 * @param OnCompleteServer Call upon completion if current instance is a dedicated server
	 * @return True if Async Task started successfully, false if task already running
	 */
	bool UpdateConnectedPlayersStats(
		const int32 LocalUserNum,
		const bool bToReset = false,
		const FOnlineStatsUpdateStatsComplete& OnCompleteClient = {},
		const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer = {});
// @@@SNIPEND
	
// @@@SNIPSTART StatsEssentialsSubsystem.h-private
// @@@MULTISNIP Interface {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP StatsDelegate {"selectedLines": ["1", "8-10"]}
// @@@MULTISNIP UpdateConnectedPlayersStatsOnGameEnds {"selectedLines": ["1-3"]}
private:
	UFUNCTION()
	void UpdateConnectedPlayersStatsOnGameEnds(const FString& Reason);

	IOnlineIdentityPtr IdentityPtr;
	FOnlineStatisticAccelBytePtr ABStatsPtr;

	FOnlineStatsQueryUsersStatsComplete OnQueryUsersStatsComplete;
	FOnlineStatsUpdateStatsComplete OnUpdateStatsComplete;
	FOnUpdateMultipleUserStatItemsComplete OnServerUpdateStatsComplete;
// @@@SNIPEND
};
