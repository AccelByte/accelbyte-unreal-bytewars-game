// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Player/AccelByteWarsPlayerController.h"

#include "Core/System/AccelByteWarsGameInstance.h"

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
