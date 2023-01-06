// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetManager/GameModes/GameModeTypeDataAsset.h"

const FPrimaryAssetType	UGameModeTypeDataAsset::GameModeTypeAssetType = TEXT("GameModeType");

FGameModeTypeData UGameModeTypeDataAsset::GetGameModeTypeDataForType(const FPrimaryAssetId& Type)
{
	FGameModeTypeData GameModeTypeData;
	GameModeTypeData.GameModeType = Type;
	GameModeTypeData.DisplayName = UAccelByteWarsDataAsset::GetDisplayNameForAsset(Type);
	GameModeTypeData.Description = UAccelByteWarsDataAsset::GetDescriptionForAsset(Type);
	return GameModeTypeData;
}
