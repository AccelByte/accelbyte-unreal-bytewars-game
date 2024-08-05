// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsWidgetModels.generated.h"

UENUM(BlueprintType)
enum class EBaseUIStackType : uint8
{
	Prompt UMETA(DisplayName = "Prompt"),
	FTUE UMETA(DisplayName = "FTUE"),
	PushNotification UMETA(DisplayName = "Push Notification"),
	Menu UMETA(DisplayName = "Menu"),
	InGameMenu UMETA(DisplayName = "In-Game Menu"),
	InGameHUD UMETA(DisplayName = "In-Game HUD")
};
ENUM_RANGE_BY_FIRST_AND_LAST(EBaseUIStackType, EBaseUIStackType::Prompt, EBaseUIStackType::InGameHUD);

UENUM(BlueprintType)
enum class EAccelByteWarsWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};
ENUM_RANGE_BY_FIRST_AND_LAST(EAccelByteWarsWidgetInputMode, EAccelByteWarsWidgetInputMode::Default, EAccelByteWarsWidgetInputMode::Menu);