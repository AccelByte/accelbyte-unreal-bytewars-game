// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "AccelByteWarsInGameCameraActor.generated.h"

class AAccelByteWarsInGameGameState;

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsInGameCameraActor : public ACameraActor
{
	GENERATED_BODY()

	AAccelByteWarsInGameCameraActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintCallable)
	void PulseBackground();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	float GetHudHeight();

private:
	void AdjustCamera();

	UPROPERTY()
	AAccelByteWarsInGameGameState* InGameGameState;

	float PulseTarget = 0.0f;
};
