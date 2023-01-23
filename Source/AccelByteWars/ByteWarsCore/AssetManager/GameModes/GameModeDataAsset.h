// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/AssetManager/AccelByteWarsDataAsset.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "GameModeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UGameModeDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UGameModeDataAsset()
	{
		AssetType = UGameModeDataAsset::GameModeAssetType;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		if (CodeName.IsEmpty())
		{
			return Super::GetPrimaryAssetId();
		}

		// Use Alias for Game Mode AssetId for easy lookup
		return UGameModeDataAsset::GenerateAssetIdFromCodeName(CodeName);
	}
	
public:
	static FGameModeData GetGameModeDataByCodeName(const FString& InCodeName);

	static FPrimaryAssetId GenerateAssetIdFromCodeName(const FString& InCodeName);
	static FString GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId);
	static FText GetDisplayNameByCodeName(const FString& InCodeName);
	static FPrimaryAssetId GetGameModeTypeForCodeName(const FString& InCodeName);

public:
	// Base Class to use for this mode
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	TSoftClassPtr<class AAccelByteWarsGameModeBase> DefaultClass;

	// Alias to set for this mode (needs to be unique)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	// String representation of GameModeType PrimaryAssetId
	// Stored as string so it is asset registry searchable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	FString GameModeTypeString;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType GameModeType = EGameModeType::FFA;
	
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
	
public:
	static const FPrimaryAssetType GameModeAssetType;
};
