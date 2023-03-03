// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameModes/AccelByteWarsGameStateBase.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameState);

void AAccelByteWarsGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsGameStateBase, LobbyStatus);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, LobbyCountdown);

	DOREPLIFETIME(AAccelByteWarsGameStateBase, TimeLeft);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, GameStatus);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, Teams);
	DOREPLIFETIME(AAccelByteWarsGameStateBase, PreGameCountdown);
}

void AAccelByteWarsGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// backup Teams data to GameInstance
	if (bAutoBackupTeamsData)
	{
		ByteWarsGameInstance->Teams = Teams;
	}
}

void AAccelByteWarsGameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UGameInstance* GameInstance = GetGameInstance();
	ByteWarsGameInstance = Cast<UAccelByteWarsGameInstance>(GameInstance);
	if (!ensure(ByteWarsGameInstance))
	{
		GAMESTATE_LOG(Warning, TEXT("Game Instance is not (derived from) UAccelByteWarsGameInstance"));
		return;
	}

	// restore Teams data from GameInstance
	if (bAutoRestoreTeamsData) Teams = ByteWarsGameInstance->Teams;
}

TArray<int32> AAccelByteWarsGameStateBase::GetRemainingTeams() const
{
	TArray<int32> RemainingTeams;
	for (const FGameplayTeamData& Team : Teams)
	{
		if (Team.GetTeamLivesLeft() > 0)
		{
			RemainingTeams.AddUnique(Team.TeamId);
		}
	}
	return RemainingTeams;
}

bool AAccelByteWarsGameStateBase::GetTeamDataByTeamId(
	const int32 TeamId,
	FGameplayTeamData& OutTeamData,
	float& OutTeamScore,
	int32& OutTeamLivesLeft,
	int32& OutTeamKillCount)
{
	if (Teams.IsValidIndex(TeamId))
	{
		OutTeamData = Teams[TeamId];
		OutTeamScore = OutTeamData.GetTeamScore();
		OutTeamLivesLeft = OutTeamData.GetTeamLivesLeft();
		OutTeamKillCount = OutTeamData.GetTeamKillCount();

		return true;
	}
	return false;
}

bool AAccelByteWarsGameStateBase::GetPlayerDataById(
	const FUniqueNetIdRepl UniqueNetId,
	FGameplayPlayerData& OutPlayerData,
	const int32 ControllerId)
{
	if (const FGameplayPlayerData* PlayerData = GetPlayerDataById(UniqueNetId, ControllerId))
	{
		OutPlayerData = *PlayerData;
		return true;
	}
	return false;
}

FGameplayPlayerData* AAccelByteWarsGameStateBase::GetPlayerDataById(
	const FUniqueNetIdRepl UniqueNetId,
	const int32 ControllerId)
{
	FGameplayPlayerData* PlayerData = nullptr;
	for (FGameplayTeamData& Team : Teams)
	{
		PlayerData = Team.TeamMembers.FindByKey(FGameplayPlayerData{UniqueNetId, ControllerId});
		if (PlayerData)
		{
			break;
		}
	}
	return PlayerData;
}

int32 AAccelByteWarsGameStateBase::GetRegisteredPlayersNum() const
{
	int32 Num = 0;
	for (const FGameplayTeamData& Team : Teams)
	{
		Num += Team.TeamMembers.Num();
	}
	return Num;
}

bool AAccelByteWarsGameStateBase::AddPlayerToTeam(
	const uint8 TeamId,
	const FUniqueNetIdRepl UniqueNetId,
	int32& OutLives,
	const int32 ControllerId,
	const float Score,
	const int32 KillCount,
	const bool bForce)
{
	// check if player have been added to any team
	FGameplayPlayerData PlayerDataTemp;
	if (!bForce && GetPlayerDataById(UniqueNetId,PlayerDataTemp, ControllerId))
	{
		GAMESTATE_LOG(Warning, TEXT("AddPlayerToTeam: Player data not found. Cancelling operation"));
		return false;
	}

	// if team not found, add until index matched TeamId. By design, TeamId represent Index in the array
	while (!Teams.IsValidIndex(TeamId))
	{
		Teams.Add(FGameplayTeamData{Teams.Num()});
	}

	// add player's data
	OutLives = OutLives == INDEX_NONE ? ByteWarsGameInstance->GameSetup.StartingLives : OutLives;
	Teams[TeamId].TeamMembers.Add(FGameplayPlayerData{
		UniqueNetId,
		ControllerId,
		TeamId,
		Score,
		KillCount,
		OutLives
	});

	return true;
}

bool AAccelByteWarsGameStateBase::RemovePlayerFromTeam(const FUniqueNetIdRepl UniqueNetId, const int32 ControllerId)
{
	bool bStatus = false;
	if (UniqueNetId.IsValid())
	{
		for (FGameplayTeamData& Team : Teams)
		{
			bStatus = Team.TeamMembers.Remove(FGameplayPlayerData{UniqueNetId, ControllerId}) > 0;
			if (bStatus) break;
		}
	}
	return bStatus;
}