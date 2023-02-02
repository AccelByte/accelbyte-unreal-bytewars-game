// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/GameModes/AccelByteWarsGameStateBase.h"

#include "Net/UnrealNetwork.h"

#define GAMESTATE_LOG(FormatString, ...) UE_LOG(LogByteWarsGameState, Log, TEXT(FormatString), __VA_ARGS__);

void AAccelByteWarsGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsGameStateBase, TimeLeft);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, bIsGameOver);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, PlayerDatas);
}

#pragma region "GameData related"
EGameModeType AAccelByteWarsGameStateBase::GetCurrentGameModeType() const
{
	return GameSetup->SelectedGameMode.SelectedGameMode.GameModeType;
}

TArray<int32> AAccelByteWarsGameStateBase::GetRemainingTeams() const
{
	TArray<int32> RemainingTeams;
	for (const FGameplayPlayerData& PlayerData : PlayerDatas)
	{
		if (PlayerData.NumLivesLeft > 0)
		{
			RemainingTeams.AddUnique(PlayerData.TeamId);
		}
	}
	return RemainingTeams;
}

UAccelByteWarsTeamSetup* AAccelByteWarsGameStateBase::GetTeamInfoByTeamId(
	const int32 TeamId,
	float& OutTeamScore,
	int32& OutTeamLivesLeft,
	TArray<FGameplayPlayerData>& OutTeamMemberDatas)
{
	UAccelByteWarsTeamSetup* TeamSetup = nullptr;
	OutTeamScore = 0.0f;
	OutTeamLivesLeft = 0;

	if (GameSetup->TeamSetups.IsValidIndex(TeamId))
	{
		// Get team setup info
		TeamSetup = GameSetup->TeamSetups[TeamId];

		// get team member data and calculate team's total score
		for (const FGameplayPlayerData& PlayerData : PlayerDatas)
		{
			if (PlayerData.TeamId == TeamId)
			{
				OutTeamMemberDatas.Add(PlayerData);
				OutTeamScore += PlayerData.Score;
				OutTeamLivesLeft += PlayerData.NumLivesLeft;
			}
		}
	}

	return TeamSetup;
}

bool AAccelByteWarsGameStateBase::AddPlayerToTeam(
	uint8 TeamId,
	const FUniqueNetIdRepl UniqueNetId,
	const FName PlayerName,
	const float Score,
	const int32 Lives,
	const int32 KillCount,
	const bool bAddToGameSetup,
	const bool bAddToPlayerData)
{
	if (!bAddToGameSetup && !bAddToPlayerData) return true;

	// if team not found, add new team
	if (TeamId >= GameSetup->TeamSetups.Num())
	{
		// check if DataAsset have been assigned
		if (!ensure(GlobalSettingsDataAsset))
		{
			GAMESTATE_LOG("GlobalSettingsDataAsset have not been set. Please set the value in the editor")
			return false;
		}

		// add new team
		UAccelByteWarsTeamSetup* TeamSetup = NewObject<UAccelByteWarsTeamSetup>();
		TeamSetup->TeamColour = GetTeamColor(TeamId);
		TeamSetup->TeamId = TeamId;
		GameSetup->TeamSetups.Add(TeamSetup);
	}

	// add to GameSetup.TeamSetup.PlayerSetup
	if (bAddToGameSetup)
	{
		UAccelByteWarsPlayerSetup* PlayerSetup = NewObject<UAccelByteWarsPlayerSetup>();
		PlayerSetup->Name = PlayerName;
		PlayerSetup->UniqueNetId = UniqueNetId;
		GameSetup->TeamSetups[TeamId]->PlayerSetups.AddUnique(PlayerSetup);
	}

	// add to PlayerDatas
	if (bAddToPlayerData)
	{
		const int32 LivesLeft = Lives == INDEX_NONE ? GameSetup->SelectedGameMode.SelectedGameMode.StartingLives : Lives;
		PlayerDatas.AddUnique(FGameplayPlayerData{
			UniqueNetId,
			TeamId,
			Score,
			KillCount,
			LivesLeft
		});
	}

	// add registered player num
	if (bAddToGameSetup && bAddToPlayerData) GameSetup->SelectedGameMode.RegisteredPlayerCount++;

	return true;
}

bool AAccelByteWarsGameStateBase::RemovePlayerFromTeam(const FUniqueNetIdRepl UniqueNetId)
{
	// remove player from TeamSetup
	bool bSuccessRemoveFromTeamSetup = false;

	for (UAccelByteWarsTeamSetup* TeamSetup : GameSetup->TeamSetups)
	{
		for (auto Iterator = TeamSetup->PlayerSetups.CreateIterator(); Iterator; ++Iterator)
		{
			if ((*Iterator)->UniqueNetId == UniqueNetId)
			{
				Iterator.RemoveCurrent();
				bSuccessRemoveFromTeamSetup = true;
				break;
			}
		}
	}

	if (!bSuccessRemoveFromTeamSetup) return false;

	// remove player from PlayerDatas
	return PlayerDatas.Remove(FGameplayPlayerData{UniqueNetId.GetUniqueNetId()}) > 0;
}

bool AAccelByteWarsGameStateBase::RemovePlayerFromTeamByTeamId(const int32 TeamId, const FUniqueNetIdRepl UniqueNetId)
{
	// remove player from TeamSetup
	bool bSuccessRemoveFromTeamSetup = false;
	if (!GameSetup->TeamSetups.IsValidIndex(TeamId)) return false;

	for (auto Iterator = GameSetup->TeamSetups[TeamId]->PlayerSetups.CreateIterator(); Iterator; ++Iterator)
	{
		if ((*Iterator)->UniqueNetId == UniqueNetId)
		{
			Iterator.RemoveCurrent();
			bSuccessRemoveFromTeamSetup = true;
			break;
		}
	}

	if (!bSuccessRemoveFromTeamSetup) return false;

	// remove player from PlayerDatas
	return PlayerDatas.Remove(FGameplayPlayerData{UniqueNetId.GetUniqueNetId()}) > 0;
}

FLinearColor AAccelByteWarsGameStateBase::GetTeamColor(uint8 TeamId) const
{
	const uint8 GlobalTeamSetupNum = GlobalSettingsDataAsset->GlobalTeamSetup.Num();
	if (TeamId >= GlobalTeamSetupNum)
	{
		// fallback: use modulo
		TeamId %= GlobalTeamSetupNum;
	}
	return GlobalSettingsDataAsset->GlobalTeamSetup[TeamId].itemColor;
}
#pragma endregion 
