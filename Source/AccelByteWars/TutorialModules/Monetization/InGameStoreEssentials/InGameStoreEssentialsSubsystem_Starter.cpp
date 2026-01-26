// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InGameStoreEssentialsSubsystem_Starter.h"

#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"

void UInGameStoreEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	ensure(Subsystem);
	StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	ensure(StoreInterface);
}

#pragma region "Tutorial"
// Put your code here.
#pragma endregion 
