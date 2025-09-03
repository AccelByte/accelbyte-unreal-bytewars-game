// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Settings/GameModeDataAssets.h"
#include "Core/Settings/SpawnerConfigurationDataAsset.h"

void FGameModeData::SetGameStyleWithString(const FString& GameStyleString)
{
	const UEnum* Enum = StaticEnum<EGameStyle>();
	Style = static_cast<EGameStyle>(Enum->GetValueByNameString(GameStyleString));
	SpawnerConfiguration = USpawnerConfigurationDataAsset::GetByCodeName(Style == EGameStyle::Frenzy ? TEXT("Frenzy") : TEXT(""));
}
