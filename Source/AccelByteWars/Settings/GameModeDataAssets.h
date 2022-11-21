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

USTRUCT(BlueprintType)
struct FGameModeTypeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	EGameModeType Type = EGameModeType::FFA;

	UPROPERTY(EditAnywhere)
	FText Description; // What is the description of this game mode
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGameModeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable)
	TSoftClassPtr<class AGameMode> DefaultClass;

	UPROPERTY(EditAnywhere)
	EGameModeType GameModeType = EGameModeType::FFA;
	
	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// Either local multiplayer game or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsLocalGame;
	
	// Either team game or not; If FFA then should be false.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsTeamGame;

	// Default team count
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TeamNum;

	// Default maximum supported player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers;

	// Default match time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MatchTime;
	
	// Default score limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreLimit;
	
};