// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegionPreferencesSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API URegionPreferencesSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
#pragma region "Tutorial"
	// put your code here
#pragma endregion
};
