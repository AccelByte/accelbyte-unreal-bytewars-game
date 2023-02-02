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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FLinearColor TeamColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 MissilesFired = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumLivesLeft = INDEX_NONE;
};
