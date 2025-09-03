// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "AccelByteWarsInGamePlayerController.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsInGamePlayerController : public AAccelByteWarsPlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void RotateShip(const float Value);

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void AdjustPower(const float Value);

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void FireMissile();

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void UsePowerUp();
	
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void ToggleMouseCapture();
};
