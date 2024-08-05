// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "InGameItemDataAsset.h"

const FPrimaryAssetType UInGameItemDataAsset::InGameItemAssetType = TEXT("InGameItemDataAsset");

UInGameItemDataAsset::UInGameItemDataAsset()
{
	AssetType = UInGameItemDataAsset::InGameItemAssetType;
}
