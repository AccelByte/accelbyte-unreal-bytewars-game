// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsEssentialsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
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
		AAccelByteWarsInGameGameMode::OnGameEndsDelegate.AddUObject(this, &ThisClass::UpdatePlayersStatOnGameEnds);
	}
}

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

bool UStatsEssentialsSubsystem::QueryLocalUserStats(
	const int32 LocalUserNum,
	const TArray<FString>& StatNames,
	const FOnlineStatsQueryUsersStatsComplete& OnComplete)
{
	const FUniqueNetIdRef LocalUserId = IdentityPtr->GetUniquePlayerId(LocalUserNum).ToSharedRef();
	return QueryUserStats(LocalUserNum, {LocalUserId}, StatNames, OnComplete);
}

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

bool UStatsEssentialsSubsystem::ResetConnectedUsersStats(
	const int32 LocalUserNum,
	const FOnlineStatsUpdateStatsComplete& OnCompleteClient,
	const FOnUpdateMultipleUserStatItemsComplete& OnCompleteServer)
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!ensure(GameState))
	{
		return false;
	}

	AAccelByteWarsGameState* ABGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!ensure(ABGameState))
	{
		return false;
	}

	// Updated stats builder. Update only existing player -> use PlayerArray
	TArray<FOnlineStatsUserUpdatedStats> UpdatedUsersStats;
	for (const TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray)
	{
		AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
		if (!ABPlayerState)
		{
			continue;
		}

		const FUniqueNetIdRepl& PlayerUniqueId = PlayerState->GetUniqueId();
		if (!PlayerUniqueId.IsValid())
		{
			continue;
		}

		FOnlineStatsUserUpdatedStats UpdatedUserStats(PlayerUniqueId->AsShared());
		if (IsRunningDedicatedServer())
		{
			// server side stats, need to be set by the server
			UpdatedUserStats.Stats.Add(StatsCode_HighestElimination, FOnlineStatUpdate{0.0f, FOnlineStatUpdate::EOnlineStatModificationType::Set});
			UpdatedUserStats.Stats.Add(StatsCode_HighestTeamDeathMatch, FOnlineStatUpdate{0.0f, FOnlineStatUpdate::EOnlineStatModificationType::Set});
			UpdatedUserStats.Stats.Add(StatsCode_KillCount, FOnlineStatUpdate{0.0f, FOnlineStatUpdate::EOnlineStatModificationType::Set});
		}
		else
		{
			// client side stats, need to be set by the client
			UpdatedUserStats.Stats.Add(StatsCode_HighestSinglePlayer, FOnlineStatUpdate{0.0f, FOnlineStatUpdate::EOnlineStatModificationType::Set});
		}
	}

	// Update stats
	return UpdateUsersStats(LocalUserNum, UpdatedUsersStats, OnCompleteClient, OnCompleteServer);
}

void UStatsEssentialsSubsystem::UpdatePlayersStatOnGameEnds()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!ensure(GameState))
	{
		return;
	}

	AAccelByteWarsGameState* ABGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!ensure(ABGameState))
	{
		return;
	}

	// Updated stats builder. Update only existing player -> use PlayerArray
	TArray<FOnlineStatsUserUpdatedStats> UpdatedUsersStats;
	for (const TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray)
	{
		AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
		if (!ABPlayerState)
		{
			continue;
		}

		const FUniqueNetIdRepl& PlayerUniqueId = PlayerState->GetUniqueId();
		if (!PlayerUniqueId.IsValid())
		{
			continue;
		}

		FOnlineStatsUserUpdatedStats UpdatedUserStats(PlayerUniqueId->AsShared());

		TTuple<FString, FOnlineStatUpdate> StatHighest;
		TTuple<FString, FOnlineStatUpdate> StatKillCount;

		if (ABGameState->GameSetup.NetworkType == EGameModeNetworkType::LOCAL)
		{
			StatHighest.Key = StatsCode_HighestSinglePlayer;
		}
		else
		{
			switch (ABGameState->GameSetup.GameModeType)
			{
			case EGameModeType::FFA:
				StatHighest.Key = StatsCode_HighestElimination;
				break;
			case EGameModeType::TDM:
				StatHighest.Key = StatsCode_HighestTeamDeathMatch;
				break;
			default: ;
			}

			StatKillCount.Key = StatsCode_KillCount;
			StatKillCount.Value = FOnlineStatUpdate{ABPlayerState->KillCount, FOnlineStatUpdate::EOnlineStatModificationType::Sum};
			UpdatedUserStats.Stats.Add(StatKillCount);
		}

		FGameplayTeamData TeamData;
		float TeamScore;
		int32 TeamTotalLives;
		int32 TeamTotalKillCount;
		ABGameState->GetTeamDataByTeamId(ABPlayerState->TeamId, TeamData, TeamScore, TeamTotalLives, TeamTotalKillCount);
		StatHighest.Value = FOnlineStatUpdate{TeamScore, FOnlineStatUpdate::EOnlineStatModificationType::Largest};
		UpdatedUserStats.Stats.Add(StatHighest);

		UpdatedUsersStats.Add(UpdatedUserStats);
	}

	// Update stats
	UpdateUsersStats(0, UpdatedUsersStats);
}
