// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/AssetManager/AccelByteWarsDataAsset.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "GameModeTypeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UGameModeTypeDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UGameModeTypeDataAsset()
	{
		AssetType = UGameModeTypeDataAsset::GameModeTypeAssetType;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(UGameModeTypeDataAsset::GameModeTypeAssetType, GetFName());
	}
public:
	static FGameModeTypeData GetGameModeTypeDataForType(const FPrimaryAssetId& InType);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameModeType Type = EGameModeType::FFA;
	
public:
	static const FPrimaryAssetType GameModeTypeAssetType;
};
