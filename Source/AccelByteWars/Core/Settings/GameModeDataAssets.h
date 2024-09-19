// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameModeDataAssets.generated.h"

// Game mode enum
UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	FFA = 0, /* Elimination */
	TDM /* Team Deathmatch */
};

// Game mode networking mode
UENUM(BlueprintType)
enum class EGameModeNetworkType : uint8
{
	DS = 0,
	P2P,
	LOCAL
};

USTRUCT(BlueprintType)
struct FGameModeTypeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType Type = EGameModeType::FFA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId GameModeType = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText::FromString(TEXT("Free For All"));
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description; // What is the description of this game mode
};

USTRUCT(BlueprintType)
struct FGameModeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType GameModeType = EGameModeType::FFA;
	
	// Game mode alias; Used for online integration.
	// DO NOT use "CustomGame" as the code name, as it is used to flag an actual custom game
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
	
	// Network type
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeNetworkType NetworkType = EGameModeNetworkType::LOCAL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsTeamGame = false;

	// Default max team count
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxTeamNum = 4;

	// Default maximum supported player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers = 1;

	// Default match time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MatchTime = 180;

	// Countdown used to start the game when on Lobby. Set to -1 to disable. Set to 0 to immediately starts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartGameCountdown = 30;

	// Lobby countdown used to shut the game down when the session has started and there's no player inside 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition="NetworkType == EGameModeNetworkType::DS"))
	int32 NotEnoughPlayerShutdownLobbyCountdown = 15;

	// Countdown used to shut the game down when the game ends. Set to -1 to disable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GameEndsShutdownCountdown = 30;

	// Minimum player count to prevent server to start the NotEnoughPlayerCountdown to shut the game down. Set to -1 to disable. DS only.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition="NetworkType == EGameModeNetworkType::DS"))
	int32 MinimumTeamCountToPreventAutoShutdown = 2;

	// Countdown used to shut the game down when currently connected player is less than MinimumTeamCountToPreventAutoShutdown. Set to -1 to disable. DS only.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition="NetworkType == EGameModeNetworkType::DS"))
	int32 NotEnoughPlayerShutdownCountdown = 30;
	
	// Default score limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreLimit = INDEX_NONE;

	// How many missiles per player can be fired at once
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FiredMissilesLimit = 1;

	// How many lives player will start with
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartingLives = 1;

	// Base missile score for killing an enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseScoreForKill = 500;

	// Missile score increment based on time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TimeScoreIncrement = 100;

	// Missile score delta time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeScoreDeltaTime = 0.1f;

	// Base missile score for skimming (hovering close to planets / ships)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkimInitialScore = 100;

	// Missile skim score delta time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SkimScoreDeltaTime = 0.25f;

	// Missile skim score multiplier
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SkimScoreAdditionalMultiplier = 2.0f;

	FString GetGameModeTypeString() const
	{
		return UEnum::GetValueAsString(GameModeType);
	}
	void SetGameModeTypeWithString(const FString& GameModeTypeString)
	{
		const UEnum* Enum = StaticEnum<EGameModeType>();
		GameModeType = static_cast<EGameModeType>(Enum->GetValueByNameString(GameModeTypeString));
	}

	FString GetNetworkTypeString() const
	{
		return UEnum::GetValueAsString(NetworkType);
	}
	void SetNetworkTypeWithString(const FString& NetworkTypeString)
	{
		const UEnum* Enum = StaticEnum<EGameModeNetworkType>();
		NetworkType = static_cast<EGameModeNetworkType>(Enum->GetValueByNameString(NetworkTypeString));
	}

	bool operator!() const
	{
		return CodeName.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct FGameStatsModel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FString CodeName = TEXT("");

	UPROPERTY(EditAnywhere)
	FText DisplayName = FText::GetEmpty();
};

USTRUCT(BlueprintType)
struct FGameStatsData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameModeType GameModeType = EGameModeType::FFA;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel HighestScoreStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel TotalScoreStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel MatchesPlayedStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel MatchesWonStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel KillCountStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameStatsModel DeathStats{};

	TArray<FString> GetStatsCodes() const
	{
		const TArray<FString> StatsCodes
		{
			HighestScoreStats.CodeName,
			TotalScoreStats.CodeName,
			MatchesPlayedStats.CodeName,
			MatchesWonStats.CodeName,
			KillCountStats.CodeName,
			DeathStats.CodeName
		};

		return StatsCodes;
	}

	TArray<FGameStatsModel> GetStatsModels() const
	{
		const TArray<FGameStatsModel> StatsData
		{
			HighestScoreStats,
			TotalScoreStats,
			MatchesPlayedStats,
			MatchesWonStats,
			KillCountStats,
			DeathStats
		};

		return StatsData;
	}
};

#define GAMESETUP_GameModeCode FName(TEXT("GAMEMODECODE")) /*String*/
#define GAMESETUP_IsCustomGame FName(TEXT("ISCUSTOMGAME")) /*bool*/
#define GAMESETUP_GameModeType FName(TEXT("GAMEMODETYPE")) /*String based on EGameModeType*/
#define GAMESETUP_DisplayName FName(TEXT("DISPLAYNAME")) /*String*/
#define GAMESETUP_NetworkType FName(TEXT("NETWORKTYPE")) /*String based on EGameModeNetworkType*/
#define GAMESETUP_IsTeamGame FName(TEXT("ISTEAMGAME")) /*bool*/
#define GAMESETUP_MaxTeamNum FName(TEXT("MAXTEAMNUM")) /*int32*/
#define GAMESETUP_MaxPlayers FName(TEXT("MAXPLAYERS")) /*int32*/
#define GAMESETUP_MatchTime FName(TEXT("MATCHTIME")) /*int32; -1 = no limit*/
#define GAMESETUP_StartGameCountdown FName(TEXT("STARTGAMECOUNTDOWN")) /*int32*/
#define GAMESETUP_GameEndsShutdownCountdown FName(TEXT("GAMEENDSSHUTDOWNCOUNTDOWN")) /*int32*/
#define GAMESETUP_MinimumTeamCountToPreventAutoShutdown FName(TEXT("MINIMUMTEAMCOUNTTOPREVENTAUTOSHUTDOWN")) /*int32*/
#define GAMESETUP_NotEnoughPlayerShutdownCountdown FName(TEXT("NOTENOUGHPLAYERSHUTDOWNCOUNTDOWN")) /*int32*/
#define GAMESETUP_ScoreLimit FName(TEXT("SCORELIMIT")) /*int32*/
#define GAMESETUP_FiredMissilesLimit FName(TEXT("FIREDMISSILESLIMIT")) /*int32; -1 = no limit*/
#define GAMESETUP_StartingLives FName(TEXT("STARTINGLIVES")) /*int32*/
#define GAMESETUP_BaseScoreForKill FName(TEXT("BASESCOREFORKILL")) /*int32*/
#define GAMESETUP_TimeScoreIncrement FName(TEXT("TIMESCOREINCREMENT")) /*int32*/
#define GAMESETUP_TimeScoreDeltaTime FName(TEXT("TIMESCOREDELTATIME")) /*int32*/
#define GAMESETUP_SkimInitialScore FName(TEXT("SKIMINITIALSCORE")) /*int32*/
#define GAMESETUP_SkimScoreDeltaTime FName(TEXT("SKIMSCOREDELTATIME")) /*int32*/
#define GAMESETUP_SkimScoreAdditionalMultiplier FName(TEXT("SKIMSCOREADDITIONALMULTIPLIER")) /*int32*/
#define GAMESTATS_GameModeSinglePlayer FName(TEXT("SINGLEPLAYER")) /*String*/
#define GAMESTATS_GameModeElimination FName(TEXT("ELIMINATION")) /*String*/
#define GAMESTATS_GameModeTeamDeathmatch FName(TEXT("TEAMDEATHMATCH")) /*String*/
