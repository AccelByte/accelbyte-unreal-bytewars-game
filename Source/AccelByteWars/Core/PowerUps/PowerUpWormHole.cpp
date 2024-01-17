// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/PowerUps/PowerUpWormHole.h"

#include "Core/Player/AccelByteWarsPlayerPawn.h"

APowerUpWormHole::APowerUpWormHole()
{
	// Setup a do nothing root component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldSphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;

	// Setup shield fx
	WormHoleFx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShieldNiagaraComponent"));
	WormHoleFx->SetWorldScale3D(FVector(2.5f, 2.5f, 2.5f));
	WormHoleFx->SetupAttachment(RootComponent);
	WormHoleFx->bAutoActivate = false;

	// Setup audio component
	WormHoleAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShieldAudioComponent"));
	WormHoleAudioComponent->SetupAttachment(RootComponent);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	bReplicates = true;
}

// Called every frame
void APowerUpWormHole::Tick(float DeltaTime)
{
	if (MarkAsExpired)
	{	
		CurrentWormHoleLifetime += DeltaTime;
		if (CurrentWormHoleLifetime > MaxWormHoleLifetime)
		{
			MarkAsExpired = false;
			Server_WormHoleExpired();
		}
	}
}

void APowerUpWormHole::Server_InitiateWormHoleGenerator_Implementation()
{
	if (GetOwner() == nullptr)
		return;

	AAccelByteWarsPlayerPawn* ABPawn = Cast<AAccelByteWarsPlayerPawn>(GetOwner());
	if (ABPawn == nullptr)
		return;

	FVector NewPawnLocation = FindGoodWormHoleExitLocation();
	Multicast_InitiateWormHoleGenerator(ABPawn, NewPawnLocation);

	ABPawn->SetActorLocation(NewPawnLocation);

	MarkAsExpired = true;
}

void APowerUpWormHole::Multicast_InitiateWormHoleGenerator_Implementation(AActor* ActorToMove, FVector NewPawnLocation)
{
	if (WormHoleFx != nullptr)
		WormHoleFx->Activate(true);

	ActorToMove->SetActorLocation(NewPawnLocation);
}

FVector APowerUpWormHole::FindGoodWormHoleExitLocation()
{
	FVector Position = FVector::ZeroVector;

	const TArray<FVector> ActiveGameObjectLocations = GetActiveGameObjectsPosition(1.5f, 25.0f);
	FVector2D Position2D = FindGoodSpawnLocation(ActiveGameObjectLocations, false);
	Position.X = Position2D.X;
	Position.Y = Position2D.Y;

	return Position;
}

TArray<FVector> APowerUpWormHole::GetActiveGameObjectsPosition(
	const float SeparationFactor,
	const float ShipSeparationFactor) const
{
	TArray<FVector> Positions;

	if (GetOwner() == nullptr)
		return Positions;

	// Get Game State
	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetOwner()->GetWorld()));
	if (ABGameState == nullptr)
		return Positions;

	for (const UAccelByteWarsGameplayObjectComponent* ActiveGameObject : ABGameState->ActiveGameObjects)
	{
		// failsafe
		if (!ActiveGameObject)
		{
			continue;
		}

		const FVector& ActorLocation = ActiveGameObject->GetOwner()->GetActorLocation();
		const float Z = ActiveGameObject->Radius * 100.0 *
			(ActiveGameObject->ObjectType == EGameplayObjectType::SHIP ? ShipSeparationFactor : SeparationFactor);
		Positions.Add(FVector(ActorLocation.X, ActorLocation.Y, Z));
	}

	return Positions;
}

FVector2D APowerUpWormHole::FindGoodSpawnLocation(const TArray<FVector>& ActiveGameObjectsCoords, bool ForStars /* = false */) const
{
	FVector2D OutLocation;

	if (GetOwner() == nullptr)
		return OutLocation;

	// Get Game State
	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetOwner()->GetWorld()));
	if (ABGameState == nullptr)
		return OutLocation;

	FVector2D& MaxBound = ABGameState->MaxGameBound;
	FVector2D& MinBound = ABGameState->MinGameBound;

	if (ForStars)
	{
		MaxBound = ABGameState->MaxStarsGameBound;
		MinBound = ABGameState->MinStarsGameBound;
	}

	// Calculate prohibited area size
	double AreaProhibited = 0.0;
	for (const FVector& Coord : ActiveGameObjectsCoords)
	{
		// assuming all object takes the shape of a circle with Z as the radius
		AreaProhibited += UKismetMathLibrary::Abs(UKismetMathLibrary::GetPI() * FMath::Pow(Coord.Z, 2.0));
	}

	// Calculate total play area size
	double RangeX = (MaxBound.X - MinBound.X);
	double RangeY = (MaxBound.Y - MinBound.Y);
	double AreaTotal = UKismetMathLibrary::Abs(RangeX * RangeY);

	// Calculate allowable area size
	double AreaAllowed = AreaTotal - AreaProhibited;

	// Negative value means not enough room in the game play area -> increase play area while maintaining the original ratio
	if (AreaAllowed < 0)
	{
		const double n = FMath::Pow((AreaTotal + FMath::Abs(AreaAllowed)) / AreaTotal, 0.5);
		RangeX *= n;
		RangeY *= n;

		MaxBound.X = (RangeX + MinBound.X);
		MaxBound.Y = (RangeY + MinBound.Y);

		AreaTotal += UKismetMathLibrary::Abs(RangeX * RangeY);
		AreaAllowed = AreaTotal - AreaProhibited;

		UE_LOG(LogTemp, Log, TEXT("Play area is smaller than the prohibited area. Increasing play area. New MaxBound: %s"), *MaxBound.ToString());
	}

	// Random value within the allowable area
	double RelativeLocation = UKismetMathLibrary::RandomFloatInRange(0.0f, AreaAllowed);

	OutLocation = CalculateActualCoord(RelativeLocation, MinBound, RangeX);

	// check if "overlapped" with existing objects
	bool bUpdated = true;
	while (bUpdated)
	{
		bUpdated = false;
		for (const FVector& Coord : ActiveGameObjectsCoords)
		{
			if (IsInsideCircle(OutLocation, Coord))
			{
				// Add the X axis length inside the circle on the specified Y to the RelativeLocation
				RelativeLocation += CalculateCircleLengthAlongXonY(Coord, OutLocation.Y);
				OutLocation = CalculateActualCoord(RelativeLocation, MinBound, RangeX);

				bUpdated = true;
				break;
			}
		}
	}

	return OutLocation;
}

FVector2D APowerUpWormHole::CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX) const
{
	FVector2D OutLocation;

	OutLocation.X = FMath::Fmod(RelativeLocation, RangeX) + MinBound.X;
	OutLocation.Y = (RelativeLocation / RangeX) + MinBound.Y;

	return OutLocation;
}


bool APowerUpWormHole::IsInsideCircle(const FVector2D& Target, const FVector& Circle) const
{
	const double a = FMath::Pow(FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Target.X - Circle.X, 2.0), 0.5);
	return
		Target.Y < Circle.Y + a && Target.Y > Circle.Y - a;
}

double APowerUpWormHole::CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord) const
{
	const double a = FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Ycoord - Circle.Y, 2.0);
	if (a < 0.0)
	{
		// This shouldn't happened if this is called only if IsInsideCircle return true. Just in case.
		return 0.0;
	}

	const double Xrel = FMath::Pow(a, 0.5);
	const double Xabs1 = Circle.X + Xrel;
	const double Xabs2 = Circle.X - Xrel;

	return FMath::Abs(Xabs1 - Xabs2);
}

void APowerUpWormHole::Server_WormHoleExpired_Implementation()
{
	if (WormHoleFx != nullptr)
		WormHoleFx->Deactivate();

	if (WormHoleAudioComponent != nullptr)
		WormHoleAudioComponent->Stop();

	Destroy();
}

void APowerUpWormHole::DestroyPowerUp()
{
	Server_WormHoleExpired();
}