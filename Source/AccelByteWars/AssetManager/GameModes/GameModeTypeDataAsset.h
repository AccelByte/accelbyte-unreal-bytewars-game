// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/AccelByteWarsDataAsset.h"
#include "Settings/GameModeDataAssets.h"
#include "GameModeTypeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UGameModeTypeDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UGameModeTypeDataAsset() {
		AssetType = UGameModeTypeDataAsset::GameModeTypeAssetType;
	}

public:
	static FGameModeTypeData GetGameModeTypeDataForType(const FPrimaryAssetId& Type);
	
public:
	static const FPrimaryAssetType GameModeTypeAssetType;
};
