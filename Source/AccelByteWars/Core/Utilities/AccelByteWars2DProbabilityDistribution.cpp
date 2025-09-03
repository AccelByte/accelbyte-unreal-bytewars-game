// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWars2DProbabilityDistribution.h"
#include "Math/UnrealMathUtility.h"

AccelByteWars2DProbabilityDistribution::AccelByteWars2DProbabilityDistribution(const FVector2D& InMinPosition, const FVector2D& InMaxPosition, int32 InNumCols)
{
	NumCols = InNumCols;
	CellSize.X = (InMaxPosition.X - InMinPosition.X) / static_cast<float>(NumCols);
	NumRows = FMath::RoundToInt((InMaxPosition.Y - InMinPosition.Y) / CellSize.X);
	CellSize.Y = (InMaxPosition.Y - InMinPosition.Y) / static_cast<float>(NumRows);
	MinPosition = InMinPosition;

	Grid.SetNumZeroed(NumCols * NumRows);
}

void AccelByteWars2DProbabilityDistribution::IterateGrid(TFunctionRef<void(int32, int32, float&)> Callback)
{
	for (int32 Col = 0; Col < NumCols; Col++)
	{
		for (int32 Row = 0; Row < NumRows; Row++)
		{
			Callback(Col, Row, Grid[Row * NumCols + Col]);
		}
	}
}

FVector2D AccelByteWars2DProbabilityDistribution::GetCellLocation(int32 Col, int32 Row) const
{
	return CellSize * FVector2D(Col, Row) + MinPosition + (CellSize * 0.5f);
}

bool AccelByteWars2DProbabilityDistribution::FindGoodPosition(FVector2D& OutLocation, float Radius)
{
	float Sum = 0.0f;
	float CellLength = CellSize.Length();

	IterateGrid([Radius, &Sum, CellLength](int32 /*Col*/, int32 /*Row*/, float& Value) {
		if (Value > Radius + CellLength)
		{
			Sum += Value;
		}
	});

	// Algo::Accumulate(Grid, 0.0f);

	float RandScore = FMath::RandRange(0.0f, Sum);
	float CurrentScore = 0.0f;

	for (int32 Col = 0; Col < NumCols; Col++)
	{
		for (int32 Row = 0; Row < NumRows; Row++)
		{
			float Value = Grid[Row * NumCols + Col];
			if (Value > Radius + CellLength)
			{
				CurrentScore += Value;

				if (CurrentScore >= RandScore)
				{
					float Jitter = FMath::RandRange(-0.5f, 0.5f);
					OutLocation = GetCellLocation(Col, Row) + (CellSize * Jitter);
					return true;
				}
			}
		}
	}

	return false;
}