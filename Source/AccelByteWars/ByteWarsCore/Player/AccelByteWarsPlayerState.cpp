// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/Player/AccelByteWarsPlayerState.h"

#include "Net/UnrealNetwork.h"

void AAccelByteWarsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamColor);
	DOREPLIFETIME(AAccelByteWarsPlayerState, TeamId);
	DOREPLIFETIME(AAccelByteWarsPlayerState, MissilesFired);
	DOREPLIFETIME(AAccelByteWarsPlayerState, KillCount);
	DOREPLIFETIME(AAccelByteWarsPlayerState, NumLivesLeft);
}