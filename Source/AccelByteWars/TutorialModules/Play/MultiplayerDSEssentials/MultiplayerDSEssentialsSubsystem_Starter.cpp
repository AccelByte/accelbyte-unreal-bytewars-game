// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystem_Starter.h"

#include "MultiplayerDSEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameSession.h"

void UMultiplayerDSEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// TODO: Bind delegates

	ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(GetWorld()));
	ensure(ABSessionInt);
}

void UMultiplayerDSEssentialsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind delegates
}

#pragma region "Tutorial module Multiplayer DS Essentials"
// TODO: Define your functions here
#pragma endregion 
