// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/AssetManager/AccelByteWarsDataAsset.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "GameModeDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FGameModeData_AssetRegistry
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable)
	TSoftClassPtr<class AAccelByteWarsGameModeBase> DefaultClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType GameModeType = EGameModeType::FFA;
	
	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// Game mode alias; Used for online integration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DisplayName;
	
	// Either local multiplayer game or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsLocalGame = false;
	
	// Either team game or not; If FFA then should be false.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsTeamGame = false;

	// Default team count
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TeamNum = INDEX_NONE;

	// Default maximum supported player
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers = INDEX_NONE;

	// Default match time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MatchTime = INDEX_NONE;
	
	// Default score limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreLimit = INDEX_NONE;
	
};

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UGameModeDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UGameModeDataAsset() {
		AssetType = UGameModeDataAsset::GameModeAssetType;
	}
	
public:
	static FGameModeData GetGameModeDataByCodeName(const FString& InCodeName);

	static FPrimaryAssetId GenerateAssetIdFromCodeName(const FString& InCodeName);
	static FString GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId);
	static FText GetDisplayNameByCodeName(const FString& InCodeName);
	static FPrimaryAssetId GetGameModeTypeForCodeName(const FString& InCodeName);

public:
	// Base Class to use for this mode
	UPROPERTY(EditAnywhere, AssetRegistrySearchable)
	TSoftClassPtr<class AAccelByteWarsGameModeBase> DefaultClass;

	// Alias to set for this mode (needs to be unique)
	UPROPERTY(EditAnywhere)
	FString CodeName;

	// String representation of GameModeType PrimaryAssetId
	// Stored as string so it is asset registry searchable
	UPROPERTY(AssetRegistrySearchable)
	FString GameModeType;
	
	// Either local multiplayer game or not
	UPROPERTY(EditAnywhere)
	bool bIsLocalGame;
	
	// Either team game or not; If FFA then should be false.
	UPROPERTY(EditAnywhere)
	bool bIsTeamGame;

	// Default team count
	UPROPERTY(EditAnywhere)
	int32 TeamNum;

	// Default maximum supported player
	UPROPERTY(EditAnywhere)
	int32 MaxPlayers;

	// Default match time
	UPROPERTY(EditAnywhere)
	int32 MatchTime;
	
	// Default score limit
	UPROPERTY(EditAnywhere)
	int32 ScoreLimit;
	
public:
	static const FPrimaryAssetType GameModeAssetType;
};