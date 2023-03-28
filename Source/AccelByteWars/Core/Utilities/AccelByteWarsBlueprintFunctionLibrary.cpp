// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

#include "Kismet/KismetMathLibrary.h"

EBPNetMode UAccelByteWarsBlueprintFunctionLibrary::GetNetMode(AActor* Actor)
{
	return static_cast<EBPNetMode>(Actor->GetNetMode());
}

bool UAccelByteWarsBlueprintFunctionLibrary::IsUniqueNetIdValid(const FUniqueNetIdRepl UniqueNetId)
{
	return UniqueNetId.IsValid();
}

FVector2D UAccelByteWarsBlueprintFunctionLibrary::FindGoodSpawnLocation(
	FVector2D& InOutMaxBound,
	FVector2D& InOutMinBound,
	const TArray<FVector>& ActiveGameObjectsCoords)
{
	// Calculate prohibited area size
	double AreaProhibited = 0.0;
	for (const FVector& Coord : ActiveGameObjectsCoords)
	{
		// assuming all object takes the shape of a circle with Z as the radius
		AreaProhibited += UKismetMathLibrary::Abs(UKismetMathLibrary::GetPI() * FMath::Pow(Coord.Z, 2.0));
	}

	// Calculate total play area size
	double RangeX = (InOutMaxBound.X - InOutMinBound.X);
	double RangeY = (InOutMaxBound.Y - InOutMinBound.Y);
	double AreaTotal = UKismetMathLibrary::Abs(RangeX * RangeY);

	// Calculate allowable area size
	double AreaAllowed = AreaTotal - AreaProhibited;

	// Negative value means not enough room in the game play area -> increase play area while maintaining the original ratio
	if (AreaAllowed < 0)
	{
		const double n = FMath::Pow((AreaTotal + FMath::Abs(AreaAllowed)) / AreaTotal, 0.5);
		RangeX *= n;
		RangeY *= n;

		InOutMaxBound.X = (RangeX + InOutMinBound.X);
		InOutMaxBound.Y = (RangeY + InOutMinBound.Y);

		AreaTotal += UKismetMathLibrary::Abs(RangeX * RangeY);
		AreaAllowed = AreaTotal - AreaProhibited;

		UE_LOG(LogTemp, Log, TEXT("Play area is smaller than the prohibited area. Increasing play area. New MaxBound: %s"), *InOutMaxBound.ToString());
	}

	// Random value within the allowable area
	double RelativeLocation = UKismetMathLibrary::RandomFloatInRange(0.0f, AreaAllowed);

	FVector2D OutLocation = CalculateActualCoord(RelativeLocation, InOutMinBound, RangeX);

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
				OutLocation = CalculateActualCoord(RelativeLocation, InOutMinBound, RangeX);

				bUpdated = true;
				break;
			}
		}
	}

	return OutLocation;
}

bool UAccelByteWarsBlueprintFunctionLibrary::IsInsideCircle(const FVector2D& Target, const FVector& Circle)
{
	const double a = FMath::Pow(FMath::Pow(Circle.Z, 2.0) - FMath::Pow(Target.X - Circle.X, 2.0), 0.5);
	return
		Target.Y < Circle.Y + a && Target.Y > Circle.Y - a;
}

double UAccelByteWarsBlueprintFunctionLibrary::CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord)
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

FVector2D UAccelByteWarsBlueprintFunctionLibrary::CalculateActualCoord(
	const double RelativeLocation,
	const FVector2D& MinBound,
	const double RangeX)
{
	FVector2D OutLocation;

	OutLocation.X = FMath::Fmod(RelativeLocation, RangeX) + MinBound.X;
	OutLocation.Y = (RelativeLocation / RangeX) + MinBound.Y;

	return OutLocation;
}
