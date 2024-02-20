// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

UENUM(BlueprintType)
enum EPowerUpSelection
{
    NONE = 0				UMETA(DisplayName = "NONE"),
    BYTE_BOMB = 1			UMETA(DisplayName = "BYTE_BOMB"),
    BYTE_SHIELD = 2			UMETA(DisplayName = "BYTE_SHIELD"),
    WORM_HOLE = 3			UMETA(DisplayName = "WORM_HOLE"),
    SPLIT_MISSILE = 4		UMETA(DisplayName = "SPLIT_MISSILE")
};