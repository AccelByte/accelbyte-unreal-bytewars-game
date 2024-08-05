// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PowerUpWidgetEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"

void UPowerUpWidgetEntry::SetValue(const FString& ItemId, const int32 Count) const
{
	if (const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAsset(ItemId))
	{
		if (ItemDataAsset->Type != EItemType::PowerUp || !ItemDataAsset->Icon)
		{
			return;
		}

		Img_PowerUp->SetBrushFromTexture(ItemDataAsset->Icon);
		Tb_PowerUpCount->SetText(FText::FromString(FString::Printf(TEXT("%dx"), Count)));
	}
}
