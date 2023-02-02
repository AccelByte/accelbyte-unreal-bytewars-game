// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AccelByteWarsBlueprintFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EBPNetMode : uint8
{
	Standalone = 0,
	DedicatedServer,
	ListenServer,
	Client,
	MAX,
};

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", meta = (ExpandEnumAsExecs = "ReturnValue", DefaultToSelf = "Actor"))
	static EBPNetMode GetNetMode(AActor* Actor);
};
