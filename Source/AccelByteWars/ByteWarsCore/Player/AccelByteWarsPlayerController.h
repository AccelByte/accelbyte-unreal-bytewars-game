// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AccelByteWarsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void TriggerServerTravel(TSoftObjectPtr<UWorld> Level);

protected:
	UFUNCTION(Reliable, Server, meta=(WorldContext="WorldContextObject"))
	void ServerTravel(const FString& Url);
};
