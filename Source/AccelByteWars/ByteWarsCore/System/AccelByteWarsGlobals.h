// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
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
	
	TArray<FGameModeData*> GetAllGameModes() const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Game Mode CodeNames"))
	TArray<FString> GetAllGameModeCodeNames() const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Game Mode by Id"))
	bool GetGameModeDataById(int32 GameModeId, UPARAM(ref) FGameModeData& OutGameModeData) const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Game Mode Code Names by type"))
	TArray<FString> GetGameModeDataByType(EGameModeType GameModeType) const;
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Game Mode by CodeName"))
	bool GetGameModeDataByCodeName(const FString& CodeName, UPARAM(ref) FGameModeData& OutGameModeData) const;
	
	TArray<FGameModeTypeData*> GetAllGameModeTypes() const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Game Mode Type Enums"))
	TArray<EGameModeType> GetAllGameModeTypesEnum() const;
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Game Mode Type Data"))
	bool GetGameModeTypeData(const EGameModeType Type, UPARAM(ref) FGameModeTypeData& OutGameModeTypeData) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	class UDataTable* GameModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	class UDataTable* GameModeTypes;
};
