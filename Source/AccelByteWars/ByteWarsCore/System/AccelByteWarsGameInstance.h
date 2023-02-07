// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "ByteWarsCore/Settings/GlobalSettingsDataAsset.h"
#include "Engine/GameInstance.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "AccelByteWarsGameInstance.generated.h"

#pragma region "Structs and data storing purpose UObject declaration"
USTRUCT(BlueprintType)
struct FGameplayPlayerData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FUniqueNetIdRepl UniqueNetId;

	/**
	 * @brief Used for local game, since LocalPlayer does not have UniqueNetId (UE 5.1.0)
	 */
	UPROPERTY(BlueprintReadWrite)
	int32 ControllerId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	float Score = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 NumLivesLeft = 1;

	bool operator==(const FGameplayPlayerData& Other) const
	{
		return UniqueNetId.IsValid() ? UniqueNetId == Other.UniqueNetId : ControllerId == Other.ControllerId;
	}
};

USTRUCT(BlueprintType)
struct FGameplayTeamData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	TArray<FGameplayPlayerData> TeamMembers;

	float GetTeamScore() const
	{
		float TotalScore = 0.0f;
		for (const FGameplayPlayerData& Player : TeamMembers)
		{
			TotalScore += Player.Score;
		}
		return TotalScore;
	}

	int32 GetTeamLivesLeft() const
	{
		int32 TotalLives = 0;
		for (const FGameplayPlayerData& Player : TeamMembers)
		{
			TotalLives += Player.NumLivesLeft;
		}
		return TotalLives;
	}

	int32 GetTeamKillCount() const
	{
		int32 TotalKillCount = 0;
		for (const FGameplayPlayerData& Player : TeamMembers)
		{
			TotalKillCount += Player.KillCount;
		}
		return TotalKillCount;
	}

	bool operator==(const FGameplayTeamData& Other) const
	{
		return TeamId == Other.TeamId && TeamMembers == Other.TeamMembers;
	}
};
#pragma endregion 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerChanged, ULocalPlayer*, LocalPlayer);
DECLARE_LOG_CATEGORY_CLASS(LogByteWarsGameInstance, Log, All);

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Attributes)
	UAccelByteWarsActivatableWidget* BaseUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attributes)
	FGameModeData GameSetup;

	/**
	 * @brief Transferring data between data - purpose. Do not use this directly. Use the one in GameState instead.
	 */
	TArray<FGameplayTeamData> Teams;

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

	/**
	 * @brief Get the currently set GameSetup GameModeType
	 * @return Game Mode type
	 */
	UFUNCTION(BlueprintCallable)
	EGameModeType GetCurrentGameModeType() const;

	/**
	 * @brief Assign game mode to GameSetup based on it's code name | will use the first GameMode in data table if not found
	 * @param CodeName Target GameMode code name
	 */
	UFUNCTION(BlueprintCallable)
	void AssignGameMode(FString CodeName);

	/**
	 * @brief Get team color specified in GlobalSettingsDataAsset
	 * @param TeamId Target TeamId
	 * @return Configured team color
	 */
	UFUNCTION(BlueprintCallable)
	FLinearColor GetTeamColor(uint8 TeamId) const;

protected:
	UPROPERTY(EditAnywhere)
	UGlobalSettingsDataAsset* GlobalSettingsDataAsset;

	UPROPERTY(EditAnywhere)
	UDataTable* GameModeDataTable;

	FGameModeData GetGameModeDataByCodeName(const FString CodeName) const;
};
