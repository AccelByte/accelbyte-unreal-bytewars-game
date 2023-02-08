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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType Type = EGameModeType::FFA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId GameModeType = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText::FromString(TEXT("Free For All"));
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	TSoftClassPtr<class AAccelByteWarsGameModeBase> DefaultClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType GameModeType = EGameModeType::FFA;
	
	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
	
	// Either local multiplayer game or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsLocalGame = false;

	// Default team count
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxTeamNum = 4;

	// Default maximum supported player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers = INDEX_NONE;

	// Default match time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MatchTime = INDEX_NONE;
	
	// Default score limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreLimit = INDEX_NONE;

	// Default fired missiles limit at a time per player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FiredMissilesLimit = 1;

	// Default starting lives for each player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartingLives = 1;

	bool operator!() const
	{
		return CodeName.IsEmpty();
	}
};
