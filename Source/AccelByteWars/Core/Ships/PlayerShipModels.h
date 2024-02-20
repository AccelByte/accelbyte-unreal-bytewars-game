// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

UENUM(BlueprintType)
enum EShipDesign
{
    TRIANGLE = 0			UMETA(DisplayName = "TRIANGLE"),
    D = 1					UMETA(DisplayName = "D"),
    DOUBLE_TRIANGLE = 2		UMETA(DisplayName = "DOUBLE_TRIANGLE"),
    GLOW_XTRA = 3			UMETA(DisplayName = "GLOW_XTRA"),
    WHITE_STAR = 4			UMETA(DisplayName = "WHITE_STAR")
};