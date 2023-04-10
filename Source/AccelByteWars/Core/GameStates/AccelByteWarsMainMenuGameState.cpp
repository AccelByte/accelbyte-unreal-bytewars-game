// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"

#include "Net/UnrealNetwork.h"

void AAccelByteWarsMainMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LobbyStatus);
	DOREPLIFETIME(ThisClass, LobbyCountdown);
}
