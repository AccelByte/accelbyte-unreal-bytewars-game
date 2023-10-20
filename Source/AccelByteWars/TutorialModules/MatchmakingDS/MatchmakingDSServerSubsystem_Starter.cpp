// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingDSServerSubsystem_Starter.h"

#include "MatchmakingDSLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/GameStates/AccelByteWarsGameState.h"

void UMatchmakingDSServerSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UMatchmakingDSOnlineSession_Starter>(BaseOnlineSession);

	// TODO: Bind your functions here.
}

void UMatchmakingDSServerSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind your functions here.
}

#pragma region "Matchmaking with DS Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion
