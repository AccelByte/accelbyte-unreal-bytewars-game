// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PServerSubsystem_Starter.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/GameStates/AccelByteWarsGameState.h"

void UMatchmakingP2PServerSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UMatchmakingP2POnlineSession_Starter>(BaseOnlineSession);

	// TODO: Bind your functions here.
}

void UMatchmakingP2PServerSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind your functions here.
}

#pragma region "Matchmaking with P2P Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion
