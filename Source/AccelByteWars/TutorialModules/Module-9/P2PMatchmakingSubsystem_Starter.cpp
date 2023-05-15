// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-9/P2PMatchmakingSubsystem_Starter.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"

void UP2PMatchmakingSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	/* When transitioning to the listen server (P2P server), the server will initialize
	 * a new Game Instance and Game State (this is Unreal Engine's default behavior).
	 * Thus, the data set up before transitioning to the listen server will be reset.
	 * Therefore, the delegate below will help to reinitialize the listen server data. */
	AAccelByteWarsGameMode::OnInitializeListenServer.AddUObject(this, &ThisClass::OnServerReceivedSession);
}

void UP2PMatchmakingSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameMode::OnInitializeListenServer.RemoveAll(this);
}

#pragma region Module.9 Function Definitions

// TODO: Add your protected Module.9 function definitions here.

#pragma region Module.9 Function Definitions