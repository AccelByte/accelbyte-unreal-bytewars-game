// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AccelByteWarsPlayerState.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerState : public APlayerState
{
	GENERATED_BODY()

	//~AActor overriden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor overriden functions

public:
	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FString AvatarURL;

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

	// Number of attempt the player was almost got killed attempt in a single-lifetime
	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumKilledAttemptInSingleLifetime = 0;

	bool bShouldKick = false;
};
