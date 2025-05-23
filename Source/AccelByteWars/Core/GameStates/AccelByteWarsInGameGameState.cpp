// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


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
	DOREPLIFETIME(ThisClass, MinStarsGameBound);
	DOREPLIFETIME(ThisClass, MaxStarsGameBound);
	DOREPLIFETIME(ThisClass, GameBoundExtendMultiplier);
}

void AAccelByteWarsInGameGameState::BeginPlay()
{
	Super::BeginPlay();

	// calculate extend play area
	const float NewHalfWidth = (FMath::Abs(MaxGameBound.X - MinGameBound.X) * (GameBoundExtendMultiplier - 1)) / 2;
	const float NewHalfHeight = (FMath::Abs(MaxGameBound.Y - MinGameBound.Y) * (GameBoundExtendMultiplier - 1)) / 2;
	MaxGameBoundExtend = {MaxGameBound.X + NewHalfWidth, MaxGameBound.Y + NewHalfHeight};
	MinGameBoundExtend = {MinGameBound.X - NewHalfWidth, MinGameBound.Y - NewHalfHeight};
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

void AAccelByteWarsInGameGameState::MulticastOnPlayerDie_Implementation(const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer)
{
	OnPlayerDieDelegate.Broadcast(DeathPlayer, DeathLocation, Killer);
}