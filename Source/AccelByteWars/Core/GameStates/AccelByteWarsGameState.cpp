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

	DOREPLIFETIME(ThisClass, SimulateServerCrashCountdown);
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

	GAMESTATE_LOG(Log, TEXT("Requested code name (%s) %s. Game mode set to GameState: %s"),
		*CodeName,
		*FString(CodeName.Equals(GameSetup.CodeName) ? "Found" : "Not Found"),
		*GameSetup.CodeName);
}

void AAccelByteWarsGameState::AssignCustomGameMode(const FOnlineSessionSettings* Setting)
{
	if (!Setting)
	{
		GAMESTATE_LOG(Log, TEXT("Setting is not valid. Cancelling operation. Current Game mode: %s"), *GameSetup.CodeName);
		return;
	}

	FGameModeData Custom;

	if (FString GameModeTypeString; Setting->Get(GAMESETUP_GameModeType, GameModeTypeString))
	{
		Custom.SetGameModeTypeWithString(GameModeTypeString);
	}

	if (FString DisplayNameString; Setting->Get(GAMESETUP_DisplayName, DisplayNameString))
	{
		Custom.DisplayName = FText::FromString(DisplayNameString);
	}

	if (FString NetworkTypeString; Setting->Get(GAMESETUP_NetworkType, NetworkTypeString))
	{
		Custom.SetNetworkTypeWithString(NetworkTypeString);
	}

	if (int32 IsTeamGameString; Setting->Get(GAMESETUP_IsTeamGame, IsTeamGameString))
	{
		Custom.bIsTeamGame = static_cast<bool>(IsTeamGameString);
	}

	if (int32 MaxTeamNum; Setting->Get(GAMESETUP_MaxTeamNum, MaxTeamNum))
	{
		Custom.MaxTeamNum = MaxTeamNum;
	}

	if (int32 MaxPlayers; Setting->Get(GAMESETUP_MaxPlayers, MaxPlayers))
	{
		Custom.MaxPlayers = MaxPlayers;
	}

	if (int32 MatchTime; Setting->Get(GAMESETUP_MatchTime, MatchTime))
	{
		Custom.MatchTime = MatchTime;
	}

	if (int32 StartGameCountdown; Setting->Get(GAMESETUP_StartGameCountdown, StartGameCountdown))
	{
		Custom.StartGameCountdown = StartGameCountdown;
	}

	if (int32 GameEndsShutdownCountdown; Setting->Get(GAMESETUP_GameEndsShutdownCountdown, GameEndsShutdownCountdown))
	{
		Custom.GameEndsShutdownCountdown = GameEndsShutdownCountdown;
	}

	if (int32 MinimumTeamCountToPreventAutoShutdown; Setting->Get(GAMESETUP_MinimumTeamCountToPreventAutoShutdown, MinimumTeamCountToPreventAutoShutdown))
	{
		Custom.MinimumTeamCountToPreventAutoShutdown = MinimumTeamCountToPreventAutoShutdown;
	}

	if (int32 NotEnoughPlayerShutdownCountdown; Setting->Get(GAMESETUP_NotEnoughPlayerShutdownCountdown, NotEnoughPlayerShutdownCountdown))
	{
		Custom.NotEnoughPlayerShutdownCountdown = NotEnoughPlayerShutdownCountdown;
	}

	if (int32 ScoreLimit; Setting->Get(GAMESETUP_ScoreLimit, ScoreLimit))
	{
		Custom.ScoreLimit = ScoreLimit;
	}

	if (int32 FiredMissilesLimit; Setting->Get(GAMESETUP_FiredMissilesLimit, FiredMissilesLimit))
	{
		Custom.FiredMissilesLimit = FiredMissilesLimit;
	}

	if (int32 StartingLives; Setting->Get(GAMESETUP_StartingLives, StartingLives))
	{
		Custom.StartingLives = StartingLives;
	}

	if (int32 BaseScoreForKill; Setting->Get(GAMESETUP_BaseScoreForKill, BaseScoreForKill))
	{
		Custom.BaseScoreForKill = BaseScoreForKill;
	}

	if (int32 TimeScoreIncrement; Setting->Get(GAMESETUP_TimeScoreIncrement, TimeScoreIncrement))
	{
		Custom.TimeScoreIncrement = TimeScoreIncrement;
	}

	if (int32 TimeScoreDeltaTime; Setting->Get(GAMESETUP_TimeScoreDeltaTime, TimeScoreDeltaTime))
	{
		Custom.TimeScoreDeltaTime = TimeScoreDeltaTime;
	}

	if (int32 SkimInitialScore; Setting->Get(GAMESETUP_SkimInitialScore, SkimInitialScore))
	{
		Custom.SkimInitialScore = SkimInitialScore;
	}

	if (int32 SkimScoreDeltaTime; Setting->Get(GAMESETUP_SkimScoreDeltaTime, SkimScoreDeltaTime))
	{
		Custom.SkimScoreDeltaTime = SkimScoreDeltaTime;
	}

	if (float SkimScoreAdditionalMultiplier; Setting->Get(GAMESETUP_SkimScoreAdditionalMultiplier, SkimScoreAdditionalMultiplier))
	{
		Custom.SkimScoreAdditionalMultiplier = SkimScoreAdditionalMultiplier;
	}

	GameSetup = Custom;
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
				if (HasAuthority())
				{
					OnNotify_Teams();
				}

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