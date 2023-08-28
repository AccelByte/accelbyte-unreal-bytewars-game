// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameStates/AccelByteWarsInGameGameState.h"

#include "Net/UnrealNetwork.h"

void AAccelByteWarsInGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveGameObjects);
	DOREPLIFETIME(ThisClass, PreGameCountdown);
	DOREPLIFETIME(ThisClass, PostGameCountdown);
	DOREPLIFETIME(ThisClass, NotEnoughPlayerCountdown);
	DOREPLIFETIME(ThisClass, TimeLeft);
	DOREPLIFETIME(ThisClass, GameStatus);
	DOREPLIFETIME(ThisClass, MinGameBound);
	DOREPLIFETIME(ThisClass, MaxGameBound);
}

bool AAccelByteWarsInGameGameState::HasGameStarted() const
{
	bool bStarted = false;

	switch (GameStatus)
	{
	case EGameStatus::GAME_STARTED:
	case EGameStatus::AWAITING_PLAYERS_MID_GAME:
	case EGameStatus::GAME_ENDS_DELAY:
	case EGameStatus::GAME_ENDS:
		bStarted = true;
		break;
	default: ;
	}

	return bStarted;
}

bool AAccelByteWarsInGameGameState::HasGameEnded() const
{
	bool bEnded = false;

	switch (GameStatus)
	{
	case EGameStatus::GAME_ENDS_DELAY:
	case EGameStatus::GAME_ENDS:
		bEnded = true;
		break;
	default: ;
	}

	return bEnded;
}
