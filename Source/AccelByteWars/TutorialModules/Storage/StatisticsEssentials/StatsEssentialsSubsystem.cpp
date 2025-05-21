// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsEssentialsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

void UStatsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());

	const IOnlineStatsPtr StatsPtr = Subsystem->GetStatsInterface();
	ensure(StatsPtr);

	ABStatsPtr = StaticCastSharedPtr<FOnlineStatisticAccelByte>(StatsPtr);
	ensure(ABStatsPtr);

	IdentityPtr = Subsystem->GetIdentityInterface();
	ensure(IdentityPtr);

	// bind delegate if module active
	if (UTutorialModuleUtility::IsTutorialModuleActive(FPrimaryAssetId{ "TutorialModule:STATSESSENTIALS" }, this))
	{
		AAccelByteWarsInGameGameMode::OnGameEndsDelegate.AddUObject(this, &ThisClass::UpdateConnectedPlayersStatsOnGameEnds);
	}
}

// @@@SNIPSTART StatsEssentialsSubsystem.cpp-UpdateUsersStats
bool UStatsEssentialsSubsystem::UpdateUsersStats(const int32 LocalUserNum,
	const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUsersStats,
	const FOnlineStatsUpdateStatsComplete& OnCompleteClient,
	const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer)
{
	// AB OSS limitation: delegate must be a class member
	if (OnUpdateStatsComplete.IsBound() || OnServerUpdateStatsComplete.IsBound())
	{
		return false;
	}

	if (IsRunningDedicatedServer())
	{
		OnServerUpdateStatsComplete = FOnUpdateMultipleUserStatItemsComplete::CreateWeakLambda(
			this, [OnCompleteServer, this](const FOnlineError& ResultState, const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
			{
				OnCompleteServer.ExecuteIfBound(ResultState, Result);
				OnServerUpdateStatsComplete.Unbind();
			});

		ABStatsPtr->UpdateStats(LocalUserNum, UpdatedUsersStats, OnServerUpdateStatsComplete);
	}
	else
	{
		OnUpdateStatsComplete = FOnlineStatsUpdateStatsComplete::CreateWeakLambda(
			this, [OnCompleteClient, this](const FOnlineError& ResultState)
			{
				OnCompleteClient.ExecuteIfBound(ResultState);
				OnUpdateStatsComplete.Unbind();
			});

		const FUniqueNetIdRef LocalUserId = IdentityPtr->GetUniquePlayerId(LocalUserNum).ToSharedRef();
		ABStatsPtr->UpdateStats(LocalUserId, UpdatedUsersStats, OnUpdateStatsComplete);
	}

	return true;
}
// @@@SNIPEND

// @@@SNIPSTART StatsEssentialsSubsystem.cpp-QueryLocalUserStats
bool UStatsEssentialsSubsystem::QueryLocalUserStats(
	const int32 LocalUserNum,
	const TArray<FString>& StatNames,
	const FOnlineStatsQueryUsersStatsComplete& OnComplete)
{
	const FUniqueNetIdRef LocalUserId = IdentityPtr->GetUniquePlayerId(LocalUserNum).ToSharedRef();
	return QueryUserStats(LocalUserNum, {LocalUserId}, StatNames, OnComplete);
}
// @@@SNIPEND

// @@@SNIPSTART StatsEssentialsSubsystem.cpp-QueryUserStats
bool UStatsEssentialsSubsystem::QueryUserStats(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& StatsUsers,
	const TArray<FString>& StatNames,
	const FOnlineStatsQueryUsersStatsComplete& OnComplete)
{
	// AB OSS limitation: delegate must be a class member
	if (OnQueryUsersStatsComplete.IsBound())
	{
		return false;
	}

	OnQueryUsersStatsComplete = FOnlineStatsQueryUsersStatsComplete::CreateWeakLambda(this, [OnComplete, this](const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
	{
		OnComplete.ExecuteIfBound(ResultState, UsersStatsResult);
		OnQueryUsersStatsComplete.Unbind();
	});

	const FUniqueNetIdRef LocalUserId = IdentityPtr->GetUniquePlayerId(LocalUserNum).ToSharedRef();
	ABStatsPtr->QueryStats(LocalUserId, StatsUsers, StatNames, OnQueryUsersStatsComplete);
	return true;
}
// @@@SNIPEND

// @@@SNIPSTART StatsEssentialsSubsystem.cpp-UpdateConnectedPlayersStats
bool UStatsEssentialsSubsystem::UpdateConnectedPlayersStats(
	const int32 LocalUserNum, 
	const bool bToReset, 
	const FOnlineStatsUpdateStatsComplete& OnCompleteClient, 
	const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer)
{
	UE_LOG_STATSESSENTIALS(Log, TEXT("Updating connected player stats to %s"), bToReset ? TEXT("reset") : TEXT("new value"));

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to update players' statistics. Game instance is invalid."));
		return false;
	}

	AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(GetWorld()->GetGameState());
	if (!GameState)
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to update players' statistics. Game state is invalid."));
		return false;
	}

	// Get valid game stats data.
	const bool bIsLocalGame = GameState->GameSetup.NetworkType == EGameModeNetworkType::LOCAL;
	bool bIsGameStatsDataValid = false;
	FGameStatsData GameStatsData{};
	if (bIsLocalGame)
	{
		bIsGameStatsDataValid = GameInstance->GetGameStatsDataById(GAMESTATS_GameModeSinglePlayer, GameStatsData);
	}
	else if (GameState->GameSetup.GameModeType == EGameModeType::FFA)
	{
		bIsGameStatsDataValid = GameInstance->GetGameStatsDataById(GAMESTATS_GameModeElimination, GameStatsData);
	}
	else if (GameState->GameSetup.GameModeType == EGameModeType::TDM)
	{
		bIsGameStatsDataValid = GameInstance->GetGameStatsDataById(GAMESTATS_GameModeTeamDeathmatch, GameStatsData);
	}

	if (!bIsGameStatsDataValid)
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to update players' statistics. No statistics data to update."));
		return false;
	}

	// Update players' stats.
	TArray<FOnlineStatsUserUpdatedStats> UpdatedUsersStats;
	const int32 WinnerTeamId = GameState->GetWinnerTeamId();
	for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
	{
		AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
		if (!ABPlayerState)
		{
			UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to update player's statistics. Player state is invalid."));
			continue;
		}

		const FUniqueNetIdRepl& PlayerUniqueId = PlayerState->GetUniqueId();
		if (!PlayerUniqueId.IsValid())
		{
			UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to update player's statistics. User ID is invalid."));
			continue;
		}

		FGameplayTeamData TeamData{};
		float TeamScore = 0;
		int32 TeamTotalLives = 0, TeamTotalKillCount = 0, TeamTotalDeaths = 0;

		/* Local gameplay only has one valid account, which is the player who logged in to the game.
		 * Thus, set the stats based on the highest team data.*/
		if (bIsLocalGame)
		{
			GameState->GetHighestTeamData(TeamScore, TeamTotalLives, TeamTotalKillCount, TeamTotalDeaths);
		}
		// Each connected player account in online gameplay is valid, so set the stats based on their respective teams.
		else 
		{
			GameState->GetTeamDataByTeamId(ABPlayerState->TeamId, TeamData, TeamScore, TeamTotalLives, TeamTotalKillCount, TeamTotalDeaths);
		}

		/* If the gameplay is local, set the winner status based on if the game ends in a draw or not.
		 * If the gameplay is online, set the winner status based on whether the team ID matches with the winning team ID.*/
		const bool bIsWinner = bIsLocalGame ? WinnerTeamId != INDEX_NONE : TeamData.TeamId == WinnerTeamId;

		FOnlineStatsUserUpdatedStats UpdatedUserStats(PlayerUniqueId->AsShared());
		// Reset statistics values to zero.
		if (bToReset)
		{
			for (const FString& Code : GameStatsData.GetStatsCodes())
			{
				UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
				{
					Code,
					FOnlineStatUpdate { 0, FOnlineStatUpdate::EOnlineStatModificationType::Set }
				});
			}
		}
		// Update statistics values.
		else 
		{
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.HighestScoreStats.CodeName,
				FOnlineStatUpdate
				{
					TeamScore,
					FOnlineStatUpdate::EOnlineStatModificationType::Largest
				}
			});
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.TotalScoreStats.CodeName,
				FOnlineStatUpdate
				{
					TeamScore,
					FOnlineStatUpdate::EOnlineStatModificationType::Sum
				}
			});
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.MatchesPlayedStats.CodeName,
				FOnlineStatUpdate
				{
					1,
					FOnlineStatUpdate::EOnlineStatModificationType::Sum
				}
			});
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.MatchesWonStats.CodeName,
				FOnlineStatUpdate
				{
					bIsWinner ? 1 : 0,
					FOnlineStatUpdate::EOnlineStatModificationType::Sum
				}
			});
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.KillCountStats.CodeName,
				FOnlineStatUpdate
				{
					TeamTotalKillCount,
					FOnlineStatUpdate::EOnlineStatModificationType::Sum
				}
			});
			UpdatedUserStats.Stats.Add(TTuple<FString, FOnlineStatUpdate>
			{
				GameStatsData.DeathStats.CodeName,
				FOnlineStatUpdate
				{
					TeamTotalDeaths,
					FOnlineStatUpdate::EOnlineStatModificationType::Sum
				}
			});
		}

		UpdatedUsersStats.Add(UpdatedUserStats);
	}

	// Update stats
	return UpdateUsersStats(LocalUserNum, UpdatedUsersStats, OnCompleteClient, OnCompleteServer);
}
// @@@SNIPEND

// @@@SNIPSTART StatsEssentialsSubsystem.cpp-UpdateConnectedPlayersStatsOnGameEnds
void UStatsEssentialsSubsystem::UpdateConnectedPlayersStatsOnGameEnds(const FString& Reason)
{
	const bool bStarted = UpdateConnectedPlayersStats(0, false);
	if (bStarted)
	{
		UE_LOG_STATSESSENTIALS(Log, TEXT("Update connected player statistics on game ends is started"));
	}
	else 
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Update connected player statistics on game ends is failed"));
	}
}
// @@@SNIPEND
