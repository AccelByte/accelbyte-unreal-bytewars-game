// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWarsMissile.h"
#include "AccelByteWarsAsteroid.generated.h"

USTRUCT(BlueprintType)
struct ACCELBYTEWARS_API FAsteroidProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	float MaxTimeAlive = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	float GravitationalConstant = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	float RotationSpeed = 45.0f;
};

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsAsteroid : public AAccelByteWarsMissile
{
	GENERATED_BODY()

public:
	AAccelByteWarsAsteroid();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid")
	FAsteroidProperties Properties;

	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void SetAsteroidProperties(const FAsteroidProperties& InProperties);

	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void ApplyAsteroidRotation(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Asteroid")
	void GenerateAsteroidProceduralMesh();

protected:
	virtual void BeginPlay() override;

private:
	FRotator InitialRotation;
	float CurrentRotation = 0.0f;
};