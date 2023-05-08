// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "TutorialModules/Module-4/StatsEssentialsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

void UStatsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());

	StatsPtr = Subsystem->GetStatsInterface();
	ensure(StatsPtr);

	ABStatsPtr = StaticCastSharedPtr<FOnlineStatisticAccelByte>(StatsPtr);

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

	/**
	 * AB OSS limitation:
	 * Updated stats value will always override the current value despite changing the EOnlineStatModificationType.
	 * Workaround -> Query first, do logic on game side, then call update.
	 */
	TArray<FUniqueNetIdRef> StatsUsers;
	TArray<FString> StatNames;
	for (const FOnlineStatsUserUpdatedStats& UserStats : UpdatedUsersStats)
	{
		StatsUsers.AddUnique(UserStats.Account);
		for (const TTuple<FString, FOnlineStatUpdate>& Stat : UserStats.Stats)
		{
			StatNames.AddUnique(Stat.Key);
		}
	}

	const bool bQuerySucceeded = QueryUserStats(LocalUserNum, StatsUsers, StatNames, FOnlineStatsQueryUsersStatsComplete::CreateWeakLambda(this, [UpdatedUsersStats, this, LocalUserNum, OnCompleteClient, OnCompleteServer](const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
	{
		if (ResultState.bSucceeded)
		{
			TArray<FOnlineStatsUserUpdatedStats> UpdatedUsersStatsCopy = UpdatedUsersStats;
			for (FOnlineStatsUserUpdatedStats& UpdatedUserStats : UpdatedUsersStatsCopy)
			{
				// loop through each stats
				for (TTuple<FString, FOnlineStatUpdate>& UpdatedStat : UpdatedUserStats.Stats)
				{
					// get current value
					FVariantData CurrentValue;
					// find user
					for (const TSharedRef<const FOnlineStatsUserStats> CurrentUserStats : UsersStatsResult)
					{
						if (CurrentUserStats->Account == UpdatedUserStats.Account)
						{
							// find stat
							for (const TTuple<FString, FVariantData>& CurrentStat : CurrentUserStats->Stats)
							{
								if (CurrentStat.Key.Equals(UpdatedStat.Key))
								{
									CurrentValue = CurrentStat.Value;
									break;
								}
							}
							break;
						}
					}

					if (CurrentValue.GetType() == EOnlineKeyValuePairDataType::Type::Empty)
					{
						// data not found, override value as is
						continue;
					}

					// data found, calculate value to set based on EOnlineStatModificationType
					float UpdatedValueFloat = 0.0f;
					switch (UpdatedStat.Value.GetType())
					{
					case EOnlineKeyValuePairDataType::Empty: break;
					case EOnlineKeyValuePairDataType::Int32:
						{
							int32 UpdatedValueInt;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueInt);
							UpdatedValueFloat = static_cast<float>(UpdatedValueInt);
							break;
						}
					case EOnlineKeyValuePairDataType::UInt32:
						{
							uint32 UpdatedValueInt;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueInt);
							UpdatedValueFloat = static_cast<float>(UpdatedValueInt);
							break;
						}
					case EOnlineKeyValuePairDataType::Int64:
						{
							int64 UpdatedValueInt;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueInt);
							UpdatedValueFloat = static_cast<float>(UpdatedValueInt);
							break;
						}
					case EOnlineKeyValuePairDataType::UInt64:
						{
							uint64 UpdatedValueInt;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueInt);
							UpdatedValueFloat = static_cast<float>(UpdatedValueInt);
							break;
						}
					case EOnlineKeyValuePairDataType::Double:
						{
							double UpdatedValueDouble;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueDouble);
							UpdatedValueFloat = static_cast<float>(UpdatedValueDouble);
							break;
						}
					case EOnlineKeyValuePairDataType::String:
						{
							FString UpdatedValueString;
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueString);
							UpdatedValueFloat = FCString::Atof(*UpdatedValueString);
							break;
						}
					case EOnlineKeyValuePairDataType::Float:
						{
							UpdatedStat.Value.GetValue().GetValue(UpdatedValueFloat);
							break;
						}
					default: ;
					}
					float QueryValueFloat;
					CurrentValue.GetValue(QueryValueFloat);

					switch (UpdatedStat.Value.GetModificationType())
					{
					case FOnlineStatUpdate::EOnlineStatModificationType::Sum:
						UpdatedStat.Value = FOnlineStatUpdate{
							(QueryValueFloat + UpdatedValueFloat), FOnlineStatUpdate::EOnlineStatModificationType::Set
						};
						break;
					case FOnlineStatUpdate::EOnlineStatModificationType::Largest:
						UpdatedStat.Value = FOnlineStatUpdate{
							(UpdatedValueFloat > QueryValueFloat ? UpdatedValueFloat : QueryValueFloat),
							FOnlineStatUpdate::EOnlineStatModificationType::Set
						};
						break;
					case FOnlineStatUpdate::EOnlineStatModificationType::Smallest:
						UpdatedStat.Value = FOnlineStatUpdate{
							(UpdatedValueFloat < QueryValueFloat ? UpdatedValueFloat : QueryValueFloat),
							FOnlineStatUpdate::EOnlineStatModificationType::Set
						};
						break;
					default:
						UpdatedStat.Value = FOnlineStatUpdate{
							UpdatedValueFloat, FOnlineStatUpdate::EOnlineStatModificationType::Set
						};;
					}
				}
			}

			// update stats (override value)
			if (IsRunningDedicatedServer())
			{
				OnServerUpdateStatsComplete = FOnUpdateMultipleUserStatItemsComplete::CreateWeakLambda(
					this, [OnCompleteServer, this](const FOnlineError& ResultState,
					                               const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
					{
						OnCompleteServer.ExecuteIfBound(ResultState, Result);
						OnServerUpdateStatsComplete.Unbind();
					});

				ABStatsPtr->UpdateStats(LocalUserNum, UpdatedUsersStatsCopy, OnServerUpdateStatsComplete);
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
				StatsPtr->UpdateStats(LocalUserId, UpdatedUsersStatsCopy, OnUpdateStatsComplete);
			}
		}
	}));

	if (!bQuerySucceeded)
	{
		return false;
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
	StatsPtr->QueryStats(LocalUserId, StatsUsers, StatNames, OnQueryUsersStatsComplete);
	return true;
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
		}

		FGameplayTeamData TeamData;
		float TeamScore;
		int32 TeamTotalLives;
		int32 TeamTotalKillCount;
		ABGameState->GetTeamDataByTeamId(ABPlayerState->TeamId, TeamData, TeamScore, TeamTotalLives, TeamTotalKillCount);
		StatHighest.Value = FOnlineStatUpdate{TeamScore, FOnlineStatUpdate::EOnlineStatModificationType::Largest};
		UpdatedUserStats.Stats.Add(StatHighest);

		TTuple<FString, FOnlineStatUpdate> StatKillCount;
		StatKillCount.Key = StatsCode_KillCount;
		StatKillCount.Value = FOnlineStatUpdate{ABPlayerState->KillCount, FOnlineStatUpdate::EOnlineStatModificationType::Sum};
		UpdatedUserStats.Stats.Add(StatKillCount);

		UpdatedUsersStats.Add(UpdatedUserStats);
	}

	// Update stats
	UpdateUsersStats(0, UpdatedUsersStats,
		FOnlineStatsUpdateStatsComplete::CreateWeakLambda(this, [](const FOnlineError& ResultState)
		{
			for (auto UpdatedUsersStat : ResultState.ErrorMessage.ToString())
			{
				
			}
		}),
		FOnUpdateMultipleUserStatItemsComplete::CreateWeakLambda(this, [](const FOnlineError& ResultState, const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
		{
			for (auto UpdatedUsersStat : Result)
			{
				
			}
		}));
}
