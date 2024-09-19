// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2POnlineSession_Starter.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineUserInterface.h"

void UMatchmakingP2POnlineSession_Starter::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// TODO: Bind your delegates here.
}

void UMatchmakingP2POnlineSession_Starter::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	// TODO: Unbind your delegates here.
}

bool UMatchmakingP2POnlineSession_Starter::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}

#pragma region "Matchmaking with P2P Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion 
