// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Templates/Function.h"

class ACCELBYTEWARS_API AccelByteWars2DProbabilityDistribution
{
public:
	AccelByteWars2DProbabilityDistribution(const FVector2D& InMinPosition, const FVector2D& InMaxPosition, int32 InNumCols);
	void IterateGrid(TFunctionRef<void(int32, int32, float&)> Callback);

	int32 GetNumCols() const { return NumCols; }
	int32 GetNumRows() const { return NumRows; }
	FVector2D GetCellSize() const { return CellSize; }
	FVector2D GetCellLocation(int32 Col, int32 Row) const;
	bool FindGoodPosition(FVector2D& OutLocation, float Radius);

private:
	int32 NumRows = 0;
	int32 NumCols = 0;
	FVector2D CellSize;
	FVector2D MinPosition;

	TArray<float> Grid;
};