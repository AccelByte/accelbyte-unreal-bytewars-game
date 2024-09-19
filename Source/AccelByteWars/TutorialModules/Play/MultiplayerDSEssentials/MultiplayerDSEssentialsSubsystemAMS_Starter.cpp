// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystemAMS_Starter.h"

#include "MultiplayerDSEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"

void UMultiplayerDSEssentialsSubsystemAMS_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// TODO: Bind delegates

	ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface());
	ensure(ABSessionInt);
}

void UMultiplayerDSEssentialsSubsystemAMS_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind delegates
}

#pragma region "Tutorial module Multiplayer DS Essentials AMS"
// TODO: Define your functions here
#pragma endregion 
