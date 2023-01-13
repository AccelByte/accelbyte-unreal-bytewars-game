// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/Player/AccelByteWarsPlayerController.h"

void AAccelByteWarsPlayerController::TriggerServerTravel(TSoftObjectPtr<UWorld> Level)
{
	const FString Url = Level.GetLongPackageName();
	ServerTravel(Url);
}

void AAccelByteWarsPlayerController::ServerTravel_Implementation(const FString& Url)
{
	GetWorld()->ServerTravel(Url);
}