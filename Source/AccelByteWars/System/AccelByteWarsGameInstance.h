// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "Settings/GlobalSettingsDataAsset.h"
#include "UI/AccelByteWarsActivatableWidget.h"


#include "AccelByteWarsGameInstance.generated.h"

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsPlayerSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	FString Name;
};

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsTeamSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	FLinearColor TeamColour;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	int TeamId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	TArray<UAccelByteWarsPlayerSetup*> PlayerSetups;
};


UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsGameSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	TArray<UAccelByteWarsTeamSetup*> TeamSetups;
};


/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsGameInstance : public UGameInstance
{
	GENERATED_BODY()



public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* BaseUIWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* MainMenuWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* HelpWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* LocalMutiplayerWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* FreeForAllWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsGameSetup* GameSetup;
};
