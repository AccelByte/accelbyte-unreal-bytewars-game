// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

#define GAME_OPTIONS_KEY FString(TEXT("GameOptions"))
#define SOUND_OPTIONS_KEY FString(TEXT("Sound"))
#define GAME_EQUIPMENT_KEY FString(TEXT("PlayerEquipment"))
#define EQUIPMENT_OPTIONS_KEY FString(TEXT("ShipEquipment"))
#define SOUND_OPTIONS_MUSIC_KEY FString(TEXT("musicvolume"))
#define SOUND_OPTIONS_SFX_KEY FString(TEXT("sfxvolume"))
#define PLAYER_EQUIPMENT_OPTIONS_SHIP_KEY FString(TEXT("shipselection"))
#define PLAYER_EQUIPMENT_OPTIONS_POWERUP_KEY FString(TEXT("powerupselection"))

DECLARE_DELEGATE_OneParam(FOnSetCloudSaveRecordComplete, bool /*bWasSuccessful*/);

DECLARE_DELEGATE_TwoParams(FOnGetCloudSaveRecordComplete, bool /*bWasSuccessful*/, FJsonObject& /*Result*/);

DECLARE_DELEGATE_OneParam(FOnDeleteCloudSaveRecordComplete, bool /*bWasSuccessful*/);