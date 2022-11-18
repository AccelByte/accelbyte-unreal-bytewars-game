// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalSettingsDataAsset.generated.h"

class UGameModeDataAssets;

// Game mode enum
UENUM(BlueprintType)
enum class EGameModeType : uint8 {
    LocalFFA,
    LocalTDM
};

USTRUCT(BlueprintType)
struct FGlobalTeamSetup {

    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor itemColor;
};

USTRUCT(BlueprintType)
struct FGlobalGameModes {

    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EGameModeType, TObjectPtr<UGameModeDataAssets>> GameModes;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ACCELBYTEWARS_API UGlobalSettingsDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGlobalTeamSetup> GlobalTeamSetup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGlobalGameModes GlobalGameModes;
};
