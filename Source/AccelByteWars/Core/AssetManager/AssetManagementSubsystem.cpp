// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/AssetManager/AssetManagementSubsystem.h"

void UAssetManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if(!AssetManager)
	{
		AssetManager = &UAccelByteWarsAssetManager::Get();
	}
}

void UAssetManagementSubsystem::Deinitialize()
{
	Super::Deinitialize();

	AssetManager = nullptr;
}