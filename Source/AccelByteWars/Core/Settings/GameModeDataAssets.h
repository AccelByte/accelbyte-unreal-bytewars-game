// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameModeDataAssets.generated.h"

// Game mode enum
UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	FFA,
	TDM
};

// Game mode networking mode
UENUM(BlueprintType)
enum class EGameModeNetworkType : uint8
{
	DS,
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// Associate third part code names to this game mode
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> ThirdPartyCodeNames;

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

#define GAMESETUP_GameModeType FName(TEXT("GAMEMODETYPE"))
#define GAMESETUP_DisplayName FName(TEXT("DISPLAYNAME"))
#define GAMESETUP_NetworkType FName(TEXT("NETWORKTYPE"))
#define GAMESETUP_IsTeamGame FName(TEXT("ISTEAMGAME"))
#define GAMESETUP_MaxTeamNum FName(TEXT("MAXTEAMNUM"))
#define GAMESETUP_MaxPlayers FName(TEXT("MAXPLAYERS"))
#define GAMESETUP_MatchTime FName(TEXT("MATCHTIME"))
#define GAMESETUP_StartGameCountdown FName(TEXT("STARTGAMECOUNTDOWN"))
#define GAMESETUP_GameEndsShutdownCountdown FName(TEXT("GAMEENDSSHUTDOWNCOUNTDOWN"))
#define GAMESETUP_MinimumTeamCountToPreventAutoShutdown FName(TEXT("MINIMUMTEAMCOUNTTOPREVENTAUTOSHUTDOWN"))
#define GAMESETUP_NotEnoughPlayerShutdownCountdown FName(TEXT("NOTENOUGHPLAYERSHUTDOWNCOUNTDOWN"))
#define GAMESETUP_ScoreLimit FName(TEXT("SCORELIMIT"))
#define GAMESETUP_FiredMissilesLimit FName(TEXT("FIREDMISSILESLIMIT"))
#define GAMESETUP_StartingLives FName(TEXT("STARTINGLIVES"))
#define GAMESETUP_BaseScoreForKill FName(TEXT("BASESCOREFORKILL"))
#define GAMESETUP_TimeScoreIncrement FName(TEXT("TIMESCOREINCREMENT"))
#define GAMESETUP_TimeScoreDeltaTime FName(TEXT("TIMESCOREDELTATIME"))
#define GAMESETUP_SkimInitialScore FName(TEXT("SKIMINITIALSCORE"))
#define GAMESETUP_SkimScoreDeltaTime FName(TEXT("SKIMSCOREDELTATIME"))
#define GAMESETUP_SkimScoreAdditionalMultiplier FName(TEXT("SKIMSCOREADDITIONALMULTIPLIER"))
