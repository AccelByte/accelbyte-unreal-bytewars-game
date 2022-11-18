// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AccelByteWarsPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	FLinearColor TeamColour;
};
