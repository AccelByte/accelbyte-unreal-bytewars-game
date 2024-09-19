// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDPlayer.generated.h"


UCLASS()
class ACCELBYTEWARS_API AHUDPlayer : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
	void UpdatePowerBarUI(FVector2D NewPosition, FLinearColor InColor);
	
};
