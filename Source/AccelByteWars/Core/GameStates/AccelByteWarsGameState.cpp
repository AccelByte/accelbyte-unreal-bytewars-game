// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsGameState.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "AccelByteWars/Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/GUICheat/GUICheatModels.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameState);

void AAccelByteWarsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GameSetup);
	DOREPLIFETIME(ThisClass, bIsServerTravelling);
	DOREPLIFETIME(ThisClass, Teams);

	DOREPLIFETIME(ThisClass, ServerCloseCountdown);
	DOREPLIFETIME(ThisClass, SimulateServerCrashCountdown);
}

void AAccelByteWarsGameState::BeginPlay()
{
	Super::BeginPlay();

	// Refresh Tutorial Module metadata based on the default object.
	if (const AAccelByteWarsGameState* DefaultObj = Cast<AAccelByteWarsGameState>(GetClass()->GetDefaultObject()))
	{
		GUICheatWidgetEntries = DefaultObj->GUICheatWidgetEntries;
	}

	InitializeGUICheatWidgetEntries();
}

void AAccelByteWarsGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeInitializeGUICheatWidgetEntries();

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
			OnNotify_GameSetup();
			OnNotify_Teams();
		}
	}

	if (OnInitialized.IsBound())
	{
		OnInitialized.Broadcast();
	}
}

void AAccelByteWarsGameState::OnNotify_IsServerTravelling() const
{
	OnIsServerTravellingChanged.Broadcast();
}

void AAccelByteWarsGameState::OnNotify_GameSetup() const
{
	OnGameSetupChanged.Broadcast();
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
		GAMESTATE_LOG(Log, TEXT("Setting is not valid. Canceling operation. Current Game mode: %s"), *GameSetup.CodeName);
		return;
	}

	GAMESTATE_LOG(Log, TEXT("Assigning custom game mode"));

	FGameModeData Custom;

	if (bool bIsCustomGame; Setting->Get(GAMESETUP_IsCustomGame, bIsCustomGame))
	{
		if (bIsCustomGame)
		{
			Custom.CodeName = TEXT("CustomGame");
		}
	}

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

	if (bool bIsTeamGame; Setting->Get(GAMESETUP_IsTeamGame, bIsTeamGame))
	{
		Custom.bIsTeamGame = bIsTeamGame;
	}

	if (double MaxTeamNum; Setting->Get(GAMESETUP_MaxTeamNum, MaxTeamNum))
	{
		Custom.MaxTeamNum = MaxTeamNum;
	}

	if (double MaxPlayers; Setting->Get(GAMESETUP_MaxPlayers, MaxPlayers))
	{
		Custom.MaxPlayers = MaxPlayers;
	}

	if (double MatchTime; Setting->Get(GAMESETUP_MatchTime, MatchTime))
	{
		Custom.MatchTime = MatchTime;
	}

	if (double StartGameCountdown; Setting->Get(GAMESETUP_StartGameCountdown, StartGameCountdown))
	{
		Custom.StartGameCountdown = StartGameCountdown;
	}

	if (double GameEndsShutdownCountdown; Setting->Get(GAMESETUP_GameEndsShutdownCountdown, GameEndsShutdownCountdown))
	{
		Custom.GameEndsShutdownCountdown = GameEndsShutdownCountdown;
	}

	if (double MinimumTeamCountToPreventAutoShutdown; Setting->Get(GAMESETUP_MinimumTeamCountToPreventAutoShutdown, MinimumTeamCountToPreventAutoShutdown))
	{
		Custom.MinimumTeamCountToPreventAutoShutdown = MinimumTeamCountToPreventAutoShutdown;
	}

	if (double NotEnoughPlayerShutdownCountdown; Setting->Get(GAMESETUP_NotEnoughPlayerShutdownCountdown, NotEnoughPlayerShutdownCountdown))
	{
		Custom.NotEnoughPlayerShutdownCountdown = NotEnoughPlayerShutdownCountdown;
	}

	if (double ScoreLimit; Setting->Get(GAMESETUP_ScoreLimit, ScoreLimit))
	{
		Custom.ScoreLimit = ScoreLimit;
	}

	if (double FiredMissilesLimit; Setting->Get(GAMESETUP_FiredMissilesLimit, FiredMissilesLimit))
	{
		Custom.FiredMissilesLimit = FiredMissilesLimit;
	}

	if (double StartingLives; Setting->Get(GAMESETUP_StartingLives, StartingLives))
	{
		Custom.StartingLives = StartingLives;
	}

	if (double BaseScoreForKill; Setting->Get(GAMESETUP_BaseScoreForKill, BaseScoreForKill))
	{
		Custom.BaseScoreForKill = BaseScoreForKill;
	}

	if (double TimeScoreIncrement; Setting->Get(GAMESETUP_TimeScoreIncrement, TimeScoreIncrement))
	{
		Custom.TimeScoreIncrement = TimeScoreIncrement;
	}

	if (double TimeScoreDeltaTime; Setting->Get(GAMESETUP_TimeScoreDeltaTime, TimeScoreDeltaTime))
	{
		Custom.TimeScoreDeltaTime = TimeScoreDeltaTime;
	}

	if (double SkimInitialScore; Setting->Get(GAMESETUP_SkimInitialScore, SkimInitialScore))
	{
		Custom.SkimInitialScore = SkimInitialScore;
	}

	if (double SkimScoreDeltaTime; Setting->Get(GAMESETUP_SkimScoreDeltaTime, SkimScoreDeltaTime))
	{
		Custom.SkimScoreDeltaTime = SkimScoreDeltaTime;
	}

	if (float SkimScoreAdditionalMultiplier; Setting->Get(GAMESETUP_SkimScoreAdditionalMultiplier, SkimScoreAdditionalMultiplier))
	{
		Custom.SkimScoreAdditionalMultiplier = SkimScoreAdditionalMultiplier;
	}

	GameSetup = Custom;
}

TArray<int32> AAccelByteWarsGameState::GetRemainingTeamIds() const
{
	TArray<int32> RemainingTeamIds;
	for (const FGameplayTeamData& Team : Teams)
	{
		if (Team.GetTeamLivesLeft() > 0)
		{
			RemainingTeamIds.AddUnique(Team.TeamId);
		}
	}
	return RemainingTeamIds;
}

TArray<int32> AAccelByteWarsGameState::GetEmptyTeamIds() const
{
	TArray<int32> EmptyTeamIds;

	// Since team is somewhat of a preset: always have the id of 0 to (MaxTeamNum - 1), if there's a number missed, treat that number as empty team id
	for (int i = 0; i < GameSetup.MaxTeamNum; ++i)
	{
		// Check if team with this ID exist
		bool bIsTeamExist = false;
		for (const FGameplayTeamData& Team : Teams)
		{
			if (Team.TeamId == i && !Team.TeamMembers.IsEmpty())
			{
				bIsTeamExist = true;
				break;
			}
		}

		if (!bIsTeamExist)
		{
			EmptyTeamIds.Add(i);
		}
	}
	return EmptyTeamIds;
}

int32 AAccelByteWarsGameState::GetWinnerTeamId() const
{
	int32 WinnerTeamId = INDEX_NONE;
	int32 HighestScore = INDEX_NONE;

	for (const FGameplayTeamData& Team : Teams)
	{
		// Empty team does not count.
		if (Team.TeamMembers.IsEmpty())
		{
			continue;
		}

		// Get winner team with highest score.
		if (Team.GetTeamScore() > HighestScore)
		{
			HighestScore = Team.GetTeamScore();
			WinnerTeamId = Team.TeamId;
		}
		// If multiple team has the same highest score, it's a draw.
		else if (Team.GetTeamScore() == HighestScore)
		{
			WinnerTeamId = INDEX_NONE;
		}
	}

	return WinnerTeamId;
}

bool AAccelByteWarsGameState::GetTeamDataByTeamId(
	const int32 TeamId,
	FGameplayTeamData& OutTeamData,
	float& OutTeamScore,
	int32& OutTeamLivesLeft,
	int32& OutTeamKillCount,
	int32& OutTeamDeaths)
{
	for (const FGameplayTeamData& Team : Teams)
	{
		if (Team.TeamId == TeamId)
		{
			OutTeamData = Team;
			OutTeamScore = OutTeamData.GetTeamScore();
			OutTeamLivesLeft = OutTeamData.GetTeamLivesLeft();
			OutTeamKillCount = OutTeamData.GetTeamKillCount();
			OutTeamDeaths = OutTeamData.GetTeamDeaths();
			return true;
		}
	}
	return false;
}

void AAccelByteWarsGameState::GetHighestTeamData(
	float& OutTeamScore,
	int32& OutTeamLivesLeft,
	int32& OutTeamKillCount,
	int32& OutTeamDeaths)
{
	OutTeamScore = 0;
	OutTeamLivesLeft = 0;
	OutTeamKillCount = 0;
	OutTeamDeaths = 0;

	for (const FGameplayTeamData& Team : Teams)
	{
		OutTeamScore = OutTeamScore < Team.GetTeamScore() ? Team.GetTeamScore() : OutTeamScore;
		OutTeamLivesLeft = OutTeamLivesLeft < Team.GetTeamLivesLeft() ? Team.GetTeamLivesLeft() : OutTeamLivesLeft;
		OutTeamKillCount = OutTeamKillCount < Team.GetTeamKillCount() ? Team.GetTeamKillCount() : OutTeamKillCount;
		OutTeamDeaths = OutTeamDeaths < Team.GetTeamDeaths() ? Team.GetTeamDeaths() : OutTeamDeaths;
	}
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
	const int32 Deaths,
	const FString PlayerName,
	const FString AvatarURL,
	const bool bForce)
{
	if (TeamId <= INDEX_NONE) 
	{
		GAMESTATE_LOG(Warning, TEXT("AddPlayerToTeam: Team ID (%d) is invalid. Canceling operation"), TeamId);
		return false;
	}

	// check if player have been added to any team
	if (FGameplayPlayerData PlayerDataTemp; !bForce && GetPlayerDataById(UniqueNetId, PlayerDataTemp, ControllerId))
	{
		GAMESTATE_LOG(Warning, TEXT("AddPlayerToTeam: Player data found. Canceling operation"));
		return false;
	}

	// Check if target team ID exist or not. If yes, add player to that team. If not, create a new team then add player to that team.
	FGameplayTeamData* TargetTeam = nullptr;
	for (int i = 0; i < Teams.Num(); ++i)
	{
		if (Teams[i].TeamId == TeamId)
		{
			TargetTeam = &Teams[i];
			break;
		}
	}
	if (!TargetTeam)
	{
		const int32 AddedTeamIndex = Teams.Add(FGameplayTeamData{TeamId});
		TargetTeam = &Teams[AddedTeamIndex];
		if (HasAuthority())
		{
			OnNotify_Teams();
		}
	}

	// Set default player name if it is empty.
	FString FinalPlayerName = PlayerName;
	if (PlayerName.IsEmpty()) 
	{
		// Set default player name based on the player index in the teams.
		if (GetNetMode() == ENetMode::NM_Standalone) 
		{
			int32 PlayerIndex = TargetTeam->TeamMembers.Num();
			for (const FGameplayTeamData& Team : Teams)
			{
				if (Team.TeamId == TeamId)
				{
					break;
				}
				PlayerIndex += Team.TeamMembers.Num();
			}

			FinalPlayerName = FText::Format(DEFAULT_PLAYER_NAME, PlayerIndex + 1).ToString();
		}
		// Set default display name from assigned delegate.
		else if (OnSetDefaultDisplayName.IsBound() && UniqueNetId.IsValid()) 
		{
			FinalPlayerName = OnSetDefaultDisplayName.Execute(UniqueNetId.GetUniqueNetId().ToSharedRef().Get());
		}
		else 
		{
			GAMESTATE_LOG(Warning, TEXT("Failed to set default display name. No handler is valid."));
		}
	}

	// Add player's data to the team
	OutLives = OutLives == INDEX_NONE ? GameSetup.StartingLives : OutLives;
	TargetTeam->TeamMembers.Add(FGameplayPlayerData{
		UniqueNetId,
		ControllerId,
		FinalPlayerName,
		AvatarURL,
		TeamId,
		Score,
		KillCount,
		Deaths,
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
	if (UniqueNetId.IsValid())
	{
		for (FGameplayTeamData& Team : Teams)
		{
			if (Team.TeamMembers.Remove(FGameplayPlayerData{UniqueNetId, ControllerId}) > 0)
			{
				if (HasAuthority())
				{
					OnNotify_Teams();
				}
				return true;
			}
		}
	}

	return false;
}

void AAccelByteWarsGameState::RemoveEmptyTeam()
{
	TArray<FGameplayTeamData> TeamDataToRemove;
	for (const FGameplayTeamData& Team : Teams)
	{
		if (Team.TeamMembers.IsEmpty())
		{
			TeamDataToRemove.Add(Team);
		}
	}

	// delete teams
	for (const FGameplayTeamData& ToRemove : TeamDataToRemove)
	{
		Teams.Remove(ToRemove);
	}
}

void AAccelByteWarsGameState::InitializeGUICheatWidgetEntries()
{
	if (!AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_GUI_CHEAT, FLAG_GUI_CHEAT_SECTION, true))
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget();
	if (!BaseUI)
	{
		return;
	}

	// Add entry
	for (UGUICheatWidgetEntry* Entry : GUICheatWidgetEntries)
	{
		if (!Entry)
		{
			GAMESTATE_LOG(Warning, TEXT("GUICheat widget entry is invalid"))
			continue;
		}

		BaseUI->AddGUICheatEntry(Entry);
	}
}

void AAccelByteWarsGameState::DeInitializeGUICheatWidgetEntries() const
{
	if (!AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_GUI_CHEAT, FLAG_GUI_CHEAT_SECTION, true))
	{
		return;
	}

	if (IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as the widget begin to tear down."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget(false);
	if (!BaseUI)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as Base UI is invalid."));
		return;
	}

	// Remove entries
	for (UGUICheatWidgetEntry* Entry : GUICheatWidgetEntries)
	{
		BaseUI->RemoveGUICheatEntry(Entry);
	}
}
