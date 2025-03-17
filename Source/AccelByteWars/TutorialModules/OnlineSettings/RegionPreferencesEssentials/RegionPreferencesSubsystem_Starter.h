// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegionPreferencesModels.h"
#include "RegionPreferencesEssentialsLog.h"
#include "OnlineSessionSettings.h"
#include "RegionPreferencesSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API URegionPreferencesSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TArray<FString> GetEnabledRegion();

	void FilterSessionSearch(TSharedRef<FOnlineSessionSearch> SessionSearch);

#pragma region "Tutorial"
	// Put your code here
#pragma endregion
};
