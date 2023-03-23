// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AccelByteWarsBlueprintFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EBPNetMode : uint8
{
	Standalone = 0,
	DedicatedServer,
	ListenServer,
	Client,
	MAX,
};

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * @brief Get Actor's Net mode
	 * @param Actor Target Actor
	 * @return NetMode
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", meta = (ExpandEnumAsExecs = "ReturnValue", DefaultToSelf = "Actor"))
	static EBPNetMode GetNetMode(AActor* Actor);

	/**
	 * @brief Get whether unique net id valid or not
	 * @param UniqueNetId Target unique net id
	 * @return whether the provided unique net id valid or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", BlueprintPure)
	static bool IsUniqueNetIdValid(const FUniqueNetIdRepl UniqueNetId);

	/**
	 * @brief Calculate random location within game's bounding area outside all active game objects radius.
	 * If bounding area < total game objects radius, then increase game's bounding area.
	 * @param InOutMaxBound Game's bounding area max coordinate
	 * @param InOutMinBound Game's bounding area min coordinate
	 * @param ActiveGameObjectsCoords Active game objects 2D coordinates (x, y) with z as it's radius
	 * @return Random location within the game's bounding area that's not within the active game objects radius
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", BlueprintPure)
	static FVector2D FindGoodSpawnLocation(
		UPARAM(ref) FVector2D& InOutMaxBound,
		UPARAM(ref) FVector2D& InOutMinBound,
		const TArray<FVector>& ActiveGameObjectsCoords);

private:
	static bool IsInsideCircle(const FVector2D& Target, const FVector& Circle);
	static double CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord);
	static FVector2D CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX);
};
