// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_CLASS(LogByteWarsGameMode, Log, All);

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	//~End of AGameModeBase interface

	protected:
	void AssignGameMode(FString CodeName) const;
};
