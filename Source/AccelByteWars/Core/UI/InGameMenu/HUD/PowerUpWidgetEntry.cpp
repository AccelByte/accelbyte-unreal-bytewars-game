// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PowerUpWidgetEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UPowerUpWidgetEntry::SetValue(const TEnumAsByte<EPowerUpSelection> PowerUp, const int32 PowerUpCount)
{
	UTexture2D* PowerUpTexture = nullptr;
	if (PowerUpTextures.IsValidIndex(PowerUp)) 
	{
		Img_PowerUp->SetBrushFromSoftTexture(PowerUpTextures[PowerUp]);
	}

	Tb_PowerUpCount->SetText(FText::FromString(FString::Printf(TEXT("%dx"), PowerUpCount)));
}