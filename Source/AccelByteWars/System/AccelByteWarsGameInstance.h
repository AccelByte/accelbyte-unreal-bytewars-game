// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "UI/AccelByteWarsActivatableWidget.h"


#include "AccelByteWarsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsGameInstance : public UGameInstance
{
	GENERATED_BODY()



public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* MainMenuWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* HelpWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* LocalMutiplayerWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* FreeForAllWidget;




};
