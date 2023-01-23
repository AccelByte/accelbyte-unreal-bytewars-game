// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AccelByteWarsAssetModels.generated.h"

USTRUCT(BlueprintType)
struct FTutorialModuleData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	TSoftClassPtr<class UAccelByteWarsActivatableWidget> DefaultUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;
	
	// Either the module is active or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsActive = true;
};