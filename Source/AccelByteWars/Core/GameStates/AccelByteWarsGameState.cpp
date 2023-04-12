// Fill out your copyright notice in the Description page of Project Settings.


#include "AccelByteWarsGameState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameState);

void AAccelByteWarsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GameSetup);
	DOREPLIFETIME(ThisClass, bIsServerTravelling);
	DOREPLIFETIME(ThisClass, Teams);
}

void AAccelByteWarsGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// backup Teams data to GameInstance
	if (bAutoBackupData)
	{
		GameInstance->Teams = Teams;
		GameInstance->GameSetup = GameSetup;
	}
}

void AAccelByteWarsGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		GAMESTATE_LOG(Warning, TEXT("Game Instance is not (derived from) UAccelByteWarsGameInstance"));
		return;
	}

	// restore data from GameInstance
	if (bAutoRestoreData)
	{
		Teams = GameInstance->Teams;
		GameSetup = GameInstance->GameSetup;
		if (HasAuthority())
		{
			OnNotify_Teams();
		}
	}
}

void AAccelByteWarsGameState::OnNotify_IsServerTravelling() const
{
	OnIsServerTravellingChanged.Broadcast();
}

void AAccelByteWarsGameState::OnNotify_Teams()
{
	OnTeamsChanged.Broadcast();
}

void AAccelByteWarsGameState::EmptyTeams()
{
	Teams.Empty();
	if (HasAuthority())
	{
		OnNotify_Teams();
	}
}

void AAccelByteWarsGameState::AssignGameMode(const FString& CodeName)
{
	GameSetup = GameInstance->GetGameModeDataByCodeName(CodeName);

	GAMESTATE_LOG(Log, TEXT("Game mode set to GameState: %s"), *GameSetup.CodeName);
}

TArray<int32> AAccelByteWarsGameState::GetRemainingTeams() const
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

bool AAccelByteWarsGameState::GetTeamDataByTeamId(
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

bool AAccelByteWarsGameState::GetPlayerDataById(
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

FGameplayPlayerData* AAccelByteWarsGameState::GetPlayerDataById(
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

int32 AAccelByteWarsGameState::GetRegisteredPlayersNum() const
{
	int32 Num = 0;
	for (const FGameplayTeamData& Team : Teams)
	{
		Num += Team.TeamMembers.Num();
	}
	return Num;
}

bool AAccelByteWarsGameState::AddPlayerToTeam(
	const int8 TeamId,
	const FUniqueNetIdRepl UniqueNetId,
	int32& OutLives,
	const int32 ControllerId,
	const float Score,
	const int32 KillCount,
	const FString PlayerName,
	const FString AvatarURL,
	const bool bForce)
{
	if (TeamId <= INDEX_NONE) 
	{
		GAMESTATE_LOG(Warning, TEXT("AddPlayerToTeam: Team Index is invalid. Cancelling operation"));
		return false;
	}

	// check if player have been added to any team
	FGameplayPlayerData PlayerDataTemp;
	if (!bForce && GetPlayerDataById(UniqueNetId,PlayerDataTemp, ControllerId))
	{
		GAMESTATE_LOG(Warning, TEXT("AddPlayerToTeam: Player data found. Cancelling operation"));
		return false;
	}

	// if team not found, add until index matched TeamId. By design, TeamId represent Index in the array
	while (!Teams.IsValidIndex(TeamId))
	{
		Teams.Add(FGameplayTeamData{Teams.Num()});
		if (HasAuthority())
		{
			OnNotify_Teams();
		}
	}

	// add player's data
	OutLives = OutLives == INDEX_NONE ? GameSetup.StartingLives : OutLives;
	Teams[TeamId].TeamMembers.Add(FGameplayPlayerData{
		UniqueNetId,
		ControllerId,
		PlayerName,
		AvatarURL,
		TeamId,
		Score,
		KillCount,
		OutLives
	});

	if (HasAuthority())
	{
		OnNotify_Teams();
	}

	return true;
}

bool AAccelByteWarsGameState::RemovePlayerFromTeam(const FUniqueNetIdRepl UniqueNetId, const int32 ControllerId)
{
	int AssignedTeamIndex = INDEX_NONE;
	bool bStatus = false;
	if (UniqueNetId.IsValid())
	{
		for (FGameplayTeamData& Team : Teams)
		{
			bStatus = Team.TeamMembers.Remove(FGameplayPlayerData{UniqueNetId, ControllerId}) > 0;
			if (bStatus) 
			{
				AssignedTeamIndex = Teams.Find(Team);
				break;
			}
		}
	}

	// Remove the team if it is empty.
	if (AssignedTeamIndex != INDEX_NONE && Teams[AssignedTeamIndex].TeamMembers.IsEmpty())
	{
		Teams.RemoveAt(AssignedTeamIndex);
		if (HasAuthority())
		{
			OnNotify_Teams();
		}
	}

	return bStatus;
}