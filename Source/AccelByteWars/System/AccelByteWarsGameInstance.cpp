// Fill out your copyright notice in the Description page of Project Settings.


#include "System/AccelByteWarsGameInstance.h"
#include "UI/GameUIManagerSubsystem.h"
#include "Player/CommonLocalPlayer.h"

int32 UAccelByteWarsGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, ControllerId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}
		
		UE_LOG(LogTemp, Log, TEXT("AddLocalPlayer: New player %s is set to ControllerId: %i"), *NewPlayer->GetName(), ControllerId);
		GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(NewPlayer));

		if(OnLocalPlayerAdded.IsBound())
		{
			OnLocalPlayerAdded.Broadcast(NewPlayer);
		}
	}
	
	return ReturnVal;
}

bool UAccelByteWarsGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: do we want to fall back to another player?
		PrimaryPlayer.Reset();
		UE_LOG(LogTemp, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UCommonLocalPlayer>(ExistingPlayer));

	if(OnLocalPlayerRemoved.IsBound())
	{
		OnLocalPlayerRemoved.Broadcast(ExistingPlayer);
	}
	
	return Super::RemoveLocalPlayer(ExistingPlayer);
}