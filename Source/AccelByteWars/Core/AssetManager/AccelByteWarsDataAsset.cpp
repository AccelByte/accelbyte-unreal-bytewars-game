// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AssetManager/AccelByteWarsDataAsset.h"

FText UAccelByteWarsDataAsset::GetDisplayNameForAsset(const FPrimaryAssetId& AssetId)
{
	return GetMetadataForAsset<FText>(AssetId, GET_MEMBER_NAME_CHECKED(UAccelByteWarsDataAsset, DisplayName));
}

FText UAccelByteWarsDataAsset::GetDescriptionForAsset(const FPrimaryAssetId& AssetId)
{
	return GetMetadataForAsset<FText>(AssetId, GET_MEMBER_NAME_CHECKED(UAccelByteWarsDataAsset, Description));
}