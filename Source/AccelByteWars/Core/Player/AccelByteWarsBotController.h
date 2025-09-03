// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AccelByteWarsBotController.generated.h"

class AAccelByteWarsPlayerPawn;

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsBotController, Log, All);

#define BOTCONTROLLER_LOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogAccelByteWarsBotController, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsBotController : public AAIController
{
	GENERATED_BODY()

public:
	AAccelByteWarsBotController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// AI behavior variables
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float ShootTimer = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float ShootInterval = 5.0f; 
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float RotationTimer = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float RotationDuration = 2.f; // giving time at the first rotation to always rotate at the target direction
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float RotationChangeInterval = 6.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MinShootInterval = 3.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MaxShootInterval = 6.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MinRotationDuration = .1f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MaxRotationDuration = .6f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MinRotationInterval = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MaxRotationInterval = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MinFirePower = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MaxFirePower = 1.0f;

	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MinAimAccDotProduct = 0.8f;
	
	UPROPERTY(EditAnywhere, Category = AccelByteWars)
	float MaxAimAccDotProduct = 9.88f;
	
	// Basic aiming variables
	UPROPERTY()
	AAccelByteWarsPlayerPawn* CurrentTarget = nullptr;
	
	float TargetUpdateTimer = 0.0f;
	float TargetUpdateInterval = 2.0f; // Update target every 2 seconds
	
	// Oscillation prevention
	bool bIsRotating = false;
	int32 LastRotationDirection = 0;
	float RotationStabilityTimer = 0.0f;
	bool bInitRotation = false;
	float CurrentAcc = 0.966f;
	
	// AI logic functions
	void UpdateAIBehavior(float DeltaTime);
	void TryRandomShoot();
	void UpdateRandomRotation();
	void SetRandomFirePower();
	
	// Basic aiming functions
	AAccelByteWarsPlayerPawn* FindNearestPlayer() const;
	void AimAtTarget(const FVector& TargetLocation);
	
	// Helper function to get the ship pawn
	AAccelByteWarsPlayerPawn* GetShipPawn() const;
};