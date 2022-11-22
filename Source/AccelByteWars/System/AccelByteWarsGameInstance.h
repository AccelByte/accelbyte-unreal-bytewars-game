// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Settings/GameModeDataAssets.h"
#include "Settings/GlobalSettingsDataAsset.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "AccelByteWarsGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerChanged, ULocalPlayer*, LocalPlayer);

USTRUCT(BlueprintType)
struct FSelectedGameMode {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameModeData SelectedGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RegisteredPlayerCount = 1; 
};

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsPlayerSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	int32 ControllerId;
};

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsTeamSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FLinearColor TeamColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	int TeamId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	TArray<UAccelByteWarsPlayerSetup*> PlayerSetups;
};


UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsGameSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	TArray<UAccelByteWarsTeamSetup*> TeamSetups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FSelectedGameMode SelectedGameMode;
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

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FSelectedGameMode SelectedGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsGameSetup* GameSetup;

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerAdded;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerRemoved;
	
private:
	/** This is the primary player*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
};
