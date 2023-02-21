// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AssetManager/GameModes/GameModeTypeDataAsset.h"

const FPrimaryAssetType	UGameModeTypeDataAsset::GameModeTypeAssetType = TEXT("GameModeType");

FGameModeTypeData UGameModeTypeDataAsset::GetGameModeTypeDataForType(const FPrimaryAssetId& InType)
{
	FGameModeTypeData GameModeTypeData;
	GameModeTypeData.GameModeType = InType;
	GameModeTypeData.Type = static_cast<EGameModeType>(UAccelByteWarsDataAsset::GetMetadataForAsset<uint8>(InType, GET_MEMBER_NAME_CHECKED(UGameModeTypeDataAsset, Type)));
	GameModeTypeData.DisplayName = UAccelByteWarsDataAsset::GetDisplayNameForAsset(InType);
	GameModeTypeData.Description = UAccelByteWarsDataAsset::GetDescriptionForAsset(InType);
	return GameModeTypeData;
}
