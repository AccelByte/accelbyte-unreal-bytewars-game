// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"

void AAccelByteWarsPlayerController::TriggerLobbyStart_Implementation()
{
	if (AAccelByteWarsGameStateBase* GameState = Cast<AAccelByteWarsGameStateBase>(GetWorld()->GetGameState()))
	{
		GameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
	}
}

void AAccelByteWarsPlayerController::TriggerServerTravel(TSoftObjectPtr<UWorld> Level)
{
	const FString Url = Level.GetLongPackageName();
	ServerTravel(Url);
}

void AAccelByteWarsPlayerController::ServerTravel_Implementation(const FString& Url)
{
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		GameInstance->bServerCurrentlyTravelling = true;
	}
	GetWorld()->ServerTravel(Url);
}