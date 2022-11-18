// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Settings/GlobalSettingsDataAsset.h"
#include "GameModeDataAssets.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ACCELBYTEWARS_API UGameModeDataAssets : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable)
	TSoftClassPtr<class AGameMode> DefaultClass;

	UPROPERTY(EditAnywhere)
	EGameModeType GameModeType = EGameModeType::LocalFFA;
	
	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Alias;

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
