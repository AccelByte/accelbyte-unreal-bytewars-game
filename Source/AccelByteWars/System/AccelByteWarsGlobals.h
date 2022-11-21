// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Settings/GameModeDataAssets.h"
#include "AccelByteWarsGlobals.generated.h"

/**
 * Stored global variables that needed by the game generally
 */
UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsGlobals : public UObject
{
	GENERATED_BODY()

// Game Mode Data
public:

	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Game Modes"))
	TArray<FGameModeData*> GetAllGameModes() const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Game Mode CodeName"))
	TArray<FString> GetAllGameModeCodeName() const;
	
	FGameModeData* GetGameModeDataById(int32 ID) const;
	
	FGameModeData* GetGameModeDataByCodeName(const FString& CodeName) const;
	TArray<FGameModeTypeData*> GetAllGameModeTypes() const;
	FGameModeTypeData* GetGameModeTypeData(const EGameModeType Type) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	class UDataTable* GameModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	class UDataTable* GameModeTypes;
};
