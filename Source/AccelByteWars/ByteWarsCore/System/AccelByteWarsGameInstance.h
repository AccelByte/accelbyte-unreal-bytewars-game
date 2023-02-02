// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "AccelByteWarsGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerChanged, ULocalPlayer*, LocalPlayer);

#pragma region "Structs and data-oriented classes declaration"
USTRUCT(BlueprintType)
struct FSelectedGameMode {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameModeData SelectedGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RegisteredPlayerCount = 0;
};

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsPlayerSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	FUniqueNetIdRepl UniqueNetId;
};

UCLASS(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsTeamSetup : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FLinearColor TeamColour;

	/**
	 * @brief Based on array index in AccelByteWarsGameSetup
	 */
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
#pragma endregion 


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsGameSetup* GameSetup;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerAdded;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerRemoved;
	
private:
	/** This is the primary player*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;

public:
	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Sounds)
	float GetMusicVolume();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Sounds)
	void SetMusicVolume(float InVolume);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Sounds)
	float GetSFXVolume();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Sounds)
	void SetSFXVolume(float InVolume);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = GameSettings)
	void LoadGameSettings();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = GameSettings)
	void SaveGameSettings();
};
