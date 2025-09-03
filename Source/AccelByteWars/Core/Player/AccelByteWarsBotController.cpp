// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsBotController.h"
#include "AccelByteWarsPlayerPawn.h"
#include "AccelByteWarsPlayerState.h"
#include "EngineUtils.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsBotController);

AAccelByteWarsBotController::AAccelByteWarsBotController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAccelByteWarsBotController::BeginPlay()
{
	Super::BeginPlay();
	
	// Randomize initial intervals for variety between bots
	ShootInterval = FMath::RandRange(MinShootInterval, MaxShootInterval);
	RotationChangeInterval = FMath::RandRange(MinRotationInterval, MaxRotationInterval);
	
	CurrentTarget = FindNearestPlayer();

	// Instantly rotating
	RotationTimer = RotationChangeInterval;
	BOTCONTROLLER_LOG(VeryVerbose, TEXT("Bot Controller started for %s"), *GetName());
}

void AAccelByteWarsBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority())
	{
		UpdateAIBehavior(DeltaTime);
	}
}

void AAccelByteWarsBotController::UpdateAIBehavior(float DeltaTime)
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return;
	}
	
	ShootTimer += DeltaTime;
	RotationTimer += DeltaTime;
	TargetUpdateTimer += DeltaTime;
	
	if (TargetUpdateTimer >= TargetUpdateInterval || CurrentTarget == nullptr)
	{
		CurrentTarget = FindNearestPlayer();
		TargetUpdateTimer = 0.0f;
	}
	
	if (ShootTimer >= ShootInterval)
	{
		TryRandomShoot();
		ShootTimer = 0.0f;
		ShootInterval = FMath::RandRange(MinShootInterval, MaxShootInterval);
	}
	
	if (RotationTimer >= RotationChangeInterval)
	{
		if(RotationTimer >= RotationChangeInterval + RotationDuration)
		{
			RotationTimer = 0.0f;
			RotationChangeInterval = FMath::RandRange(MinRotationInterval, MaxRotationInterval);
			RotationDuration = FMath::RandRange(MinRotationDuration, MaxRotationDuration);
			ShipPawn->Server_RotatePawn(0);
			bIsRotating = false;
			CurrentAcc = FMath::RandRange(MinAimAccDotProduct, MaxAimAccDotProduct);
		}
		else if(RotationTimer - RotationChangeInterval < RotationDuration)
		{
			if (IsValid(CurrentTarget))
			{
				AimAtTarget(CurrentTarget->GetActorLocation());
			}
			else
			{
				UpdateRandomRotation();
			}
		}
	}
}

void AAccelByteWarsBotController::TryRandomShoot()
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return;
	}
	
	SetRandomFirePower();
	
	ShipPawn->Server_FireMissile();
	
	BOTCONTROLLER_LOG(VeryVerbose, TEXT("Bot %s attempting to fire missile with power %f"), 
		*GetName(), ShipPawn->FirePowerLevel);
}

void AAccelByteWarsBotController::UpdateRandomRotation()
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return;
	}
	
	int32 RandomRotation = FMath::RandRange(0, 2);
	ShipPawn->Server_RotatePawn(RandomRotation);
	
	const TCHAR* RotationText = (RandomRotation == 0) ? TEXT("Stop") : 
								(RandomRotation == 1) ? TEXT("Clockwise") : TEXT("Counter-clockwise");
	BOTCONTROLLER_LOG(VeryVerbose, TEXT("Bot %s rotating: %s"), *GetName(), RotationText);
}

void AAccelByteWarsBotController::SetRandomFirePower()
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return;
	}
	
	float RandomPower = FMath::RandRange(MinFirePower, MaxFirePower);
	ShipPawn->FirePowerLevel = RandomPower;
	
	BOTCONTROLLER_LOG(VeryVerbose, TEXT("Bot %s set fire power to %f"), *GetName(), RandomPower);
}

AAccelByteWarsPlayerPawn* AAccelByteWarsBotController::FindNearestPlayer() const
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return nullptr;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	FVector ShipLocation = ShipPawn->GetActorLocation();
	AAccelByteWarsPlayerPawn* NearestPlayer = nullptr;
	float NearestDistance = FLT_MAX;
	
	// Find all player pawns in the world
	for (TActorIterator<AAccelByteWarsPlayerPawn> ActorItr(World); ActorItr; ++ActorItr)
	{
		AAccelByteWarsPlayerPawn* PlayerPawn = *ActorItr;
		if (PlayerPawn == ShipPawn || PlayerPawn->IsDestroyed)
		{
			continue;
		}
		AAccelByteWarsPlayerState* TargetPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerPawn->GetPlayerState());
		AAccelByteWarsPlayerState* BotPlayerState = GetPlayerState<AAccelByteWarsPlayerState>();
		if(TargetPlayerState && BotPlayerState
			&& TargetPlayerState->TeamId == BotPlayerState->TeamId)
		{
			continue;
		}
		
		float Distance = FVector::Dist(ShipLocation, PlayerPawn->GetActorLocation());
		
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPlayer = PlayerPawn;
		}
	}
	
	return NearestPlayer;
}

void AAccelByteWarsBotController::AimAtTarget(const FVector& TargetLocation)
{
	AAccelByteWarsPlayerPawn* ShipPawn = GetShipPawn();
	if (!ShipPawn)
	{
		return;
	}
	
	FVector BotLocation = ShipPawn->GetActorLocation();
	FVector DirectionToTarget = (TargetLocation - BotLocation).GetSafeNormal();
	
	FVector BotForward = ShipPawn->GetActorRightVector();
	
	float DotProduct = FVector::DotProduct(BotForward, DirectionToTarget);
	
	FVector CrossProduct = FVector::CrossProduct(BotForward, DirectionToTarget);
	float CrossZ = CrossProduct.Z;
	
	int32 RotationDirection = 0;
	
	// Dot product thresholds:
	// 1.0 = 0° (perfectly aligned)
	// 0.966 = 15° (close enough to stop)
	// 0.707 = 45°
	// 0.0 = 90° (perpendicular)
	if (DotProduct >= CurrentAcc)
	{
		RotationDirection = 0;
		bIsRotating = false;
	}
	else if (!bIsRotating)
	{
		if (CrossZ > 0)
		{
			RotationDirection = 2;
		}
		else
		{
			RotationDirection = 1;
		}
		
		bIsRotating = true;
		LastRotationDirection = RotationDirection;
	}
	else
	{
		RotationDirection = LastRotationDirection;
	}
	
	ShipPawn->Server_RotatePawn(RotationDirection);
	
	float AngleInDegrees = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)) * 180.0f / PI;
	
	BOTCONTROLLER_LOG(VeryVerbose, TEXT("Bot %s: Dot=%.3f, Angle=%.1f°, CrossZ=%.3f, Dir=%d"), 
		*GetName(), DotProduct, AngleInDegrees, CrossZ, RotationDirection);
}

AAccelByteWarsPlayerPawn* AAccelByteWarsBotController::GetShipPawn() const
{
	return Cast<AAccelByteWarsPlayerPawn>(GetPawn());
}