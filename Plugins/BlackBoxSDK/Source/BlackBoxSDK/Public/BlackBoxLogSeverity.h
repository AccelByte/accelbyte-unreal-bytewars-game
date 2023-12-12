// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"

#include "BlackBoxLogSeverity.generated.h"

UENUM()
enum class BlackBoxLogSeverity : uint8 {
    ERROR_ = 0x01 UMETA(DisplayName = "ERROR"),
    WARNING = 0x02 UMETA(DisplayName = "WARNING"),
    INFO = 0x04 UMETA(DisplayName = "INFO"),
    VERBOSE = 0x08 UMETA(DisplayName = "VERBOSE")
};
