// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/AccelByteWarsDataAsset.h"

FText UAccelByteWarsDataAsset::GetDisplayNameForAsset(const FPrimaryAssetId& AssetId)
{
	return GetMetadataForAsset<FText>(AssetId, GET_MEMBER_NAME_CHECKED(UAccelByteWarsDataAsset, DisplayName));
}

FText UAccelByteWarsDataAsset::GetDescriptionForAsset(const FPrimaryAssetId& AssetId)
{
	return GetMetadataForAsset<FText>(AssetId, GET_MEMBER_NAME_CHECKED(UAccelByteWarsDataAsset, Description));
}