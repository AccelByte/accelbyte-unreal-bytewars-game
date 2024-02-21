// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "PowerUpWidgetEntry.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ACCELBYTEWARS_API UPowerUpWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetValue(const TEnumAsByte<EPowerUpSelection> PowerUp, const int32 PowerUpCount);

protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UTexture2D>> PowerUpTextures;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_PowerUp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_PowerUpCount;
};
