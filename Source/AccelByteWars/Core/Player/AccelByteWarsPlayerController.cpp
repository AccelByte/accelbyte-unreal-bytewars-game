// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"
#include "GameFramework/HUD.h"

void AAccelByteWarsPlayerController::TriggerLobbyStart_Implementation()
{
	if (AAccelByteWarsGameStateBase* GameState = Cast<AAccelByteWarsGameStateBase>(GetWorld()->GetGameState()))
	{
		GameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
	}
}