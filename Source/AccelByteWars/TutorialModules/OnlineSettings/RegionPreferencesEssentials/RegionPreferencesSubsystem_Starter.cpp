// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegionPreferencesSubsystem_Starter.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Api/AccelByteQos.h"

void URegionPreferencesSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Put your code here
}

void URegionPreferencesSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// Put your code here
}

TArray<FString> URegionPreferencesSubsystem_Starter::GetEnabledRegion()
{
	TArray<FString> EnabledRegion = {};

	// Put your code here

	return EnabledRegion;
}

void URegionPreferencesSubsystem_Starter::FilterSessionSearch(TSharedRef<FOnlineSessionSearch> SessionSearch)
{
	// Put your code here
}

#pragma region "Tutorial"
// Put your code here
#pragma endregion