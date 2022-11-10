// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "UI/AccelByteWarsBaseUI.h"
#include "AccelByteWarsMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsMenuUI : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "AccelByteWars"))
	UAccelByteWarsBaseUI* GetBaseUI();
};
