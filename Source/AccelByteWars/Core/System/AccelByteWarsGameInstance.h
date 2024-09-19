// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/Settings/GlobalSettingsDataAsset.h"
#include "Engine/GameInstance.h"
#include "CommonButtonBase.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "AccelByteWarsGameInstance.generated.h"

class ULoadingWidget;
class UAccelByteWarsBaseUI;
class UAccelByteWarsActivatableWidget;
class UAccelByteWarsButtonBase;

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
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	FString AvatarURL;

	UPROPERTY(BlueprintReadWrite)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	float Score = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 NumLivesLeft = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 PowerUpCount = 0;

	// Number of attempt the player was almost got killed in a single-lifetime
	UPROPERTY(BlueprintReadWrite)
	int32 NumKilledAttemptInSingleLifetime = 0;

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

	int32 GetTeamDeaths() const
	{
		int32 TotalDeaths = 0;
		for (const FGameplayPlayerData& Player : TeamMembers)
		{
			TotalDeaths += Player.Deaths;
		}
		return TotalDeaths;
	}

	bool operator==(const FGameplayTeamData& Other) const
	{
		return TeamId == Other.TeamId && TeamMembers == Other.TeamMembers;
	}
};
#pragma endregion 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerChanged, ULocalPlayer*, LocalPlayer);
DECLARE_MULTICAST_DELEGATE(FOnGameInstanceShutdown);
DECLARE_LOG_CATEGORY_CLASS(LogAccelByteWarsGameInstance, Log, All);

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
	virtual void Shutdown() override;

#pragma region "Player Equipment"
protected:
	TMap<int32 /*player index*/, TArray<FEquippedItem>> EquippedItems;

	void UpdateEquippedItems(const int32 PlayerIndex, const TArray<FEquippedItem>& InItems);

public:

	/**
	 * @brief Get specified local player's equipped items
	 * @param PlayerIndex Target Local Player index
	 * @return Equipped items
	 */
	TArray<FEquippedItem>* GetEquippedItems(const int32 PlayerIndex);

	/**
	 * @brief Update specified local player's equipped items by providing in-game item ID.
	 * @param PlayerIndex Target Local Player index
	 * @param InGameItemIds In-game item ID and Quantity map to update player equipments to
	 */
	void UpdateEquippedItemsByInGameItemId(const int32 PlayerIndex, const TMap<FString, int32>& InGameItemIds);

	/**
	 * @brief Modify equipped item count by providing in-game item ID. Will do nothing if provided in-game item ID doesn't match the player's equipped items.
	 * @param PlayerIndex Target Local Player index
	 * @param ItemId Item ID to modify
	 * @param Modifier Set positive value to increase and negative to decrease quantity
	 */
	void ModifyEquippedItemCountByInGameItemId(const int32 PlayerIndex, const FString& ItemId, const int32 Modifier);

	/**
	 * @brief Update specified local player's equipped items by providing item SKU.
	 * @param PlayerIndex Target Local Player index
	 * @param Platform 3rd party platform where this SKU supposed to be used
	 * @param Skus Item SKU and Quantity map to update player equipments to
	 * @return True if item equipped item successfully updated, false otherwise.
	 */
	bool UpdateEquippedItemsBySku(const int32 PlayerIndex, const EItemSkuPlatform Platform, const TMap<FString, int32> Skus);

	/**
	 * @brief Remove specified item from player's equipped items.
	 * @param PlayerIndex Target Local Player index
	 * @param Platform 3rd party platform where this SKU supposed to be used
	 * @param Sku Item SKU to unequip
	 */
	void UnEquipItemBySku(const int32 PlayerIndex, const EItemSkuPlatform Platform, const FString& Sku);

	/**
	 * @brief Unequip all specified player's equipped items
	 * @param PlayerIndex Target Local Player index
	 */
	void UnEquipAll(const int32 PlayerIndex);

	/**
	 * @brief Check if specified item SKU equipped by specified player or not.
	 * @param PlayerIndex Target Local Player index
	 * @param Platform 3rd party platform where this SKU supposed to be used
	 * @param Sku Item SKU and Quantity map to update player equipments to
	 * @return True if item is equipped. False otherwise
	 */
	bool IsItemEquippedBySku(const int32 PlayerIndex, const EItemSkuPlatform Platform, const FString& Sku);
#pragma endregion 
	
public:
	/**
	 * @brief Transferring data between data - purpose. Do not use this directly. Use the one in GameState instead.
	 */
	TArray<FGameplayTeamData> Teams;
	FGameModeData GameSetup;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerAdded;

	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerChanged OnLocalPlayerRemoved;

	/**
	 * @brief Called on GameInstance::Shutdown | Just before the actual game exit. Will also be called on exit PIE.
	 */
	FOnGameInstanceShutdown OnGameInstanceShutdownDelegate;

	void OnDelayedClientTravelStarted();

	/**
	 * @brief Called on AAccelByteWarsPlayerController::DelayedClientTravel | Few seconds before the actual ClientTravel.
	 */
	FSimpleMulticastDelegate OnDelayedClientTravelStartedDelegates;

	/**
	 * @brief Return true if there's a pending connection attempt failed. Reset the pending flag when this called.
	 * @param OutFailureType Connection failure type
	 */
	bool GetIsPendingFailureNotification(ENetworkFailure::Type& OutFailureType);

	bool bIsReconnecting = false;

private:
	/** This is the primary player*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;

	ENetworkFailure::Type LastFailureType;
	FString LastFailureMessage;
	bool bPendingFailureNotification = false;

	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& Message);

public:
	virtual int32 AddLocalPlayer(ULocalPlayer* NewLocalPlayer, FPlatformUserId UserId) override;

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
	bool GetFTUEAlwaysOnSetting();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = GameSettings)
	void SetFTUEAlwaysOnSetting(bool InValue);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = GameSettings)
	void LoadGameSettings(const int32 UserIndex);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = GameSettings)
	void SaveGameSettings(const int32 UserIndex);

	void SaveGameSettings(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAccelByteWarsBaseUI* GetBaseUIWidget(bool bAutoActivate = true);

	UFUNCTION(BlueprintPure)
	TSubclassOf<UAccelByteWarsButtonBase> GetDefaultButtonClass() const { return DefaultButtonClass; }

	UFUNCTION(BlueprintPure)
	FSlateSound GetDefaultButtonClickSound() const { return DefaultButtonClickSound; }

	FGameModeData GetGameModeDataByCodeName(const FString CodeName) const;

	bool GetGameStatsDataById(const FName& Id, FGameStatsData& OutGameStatsData) const;

	/**
	 * @brief Get team color specified in GlobalSettingsDataAsset
	 * @param TeamId Target TeamId
	 * @return Configured team color
	 */
	UFUNCTION(BlueprintCallable)
	FLinearColor GetTeamColor(uint8 TeamId) const;

#pragma region "Online Session Modules implementation"
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
#pragma endregion 

protected:
	UPROPERTY()
	UAccelByteWarsBaseUI* BaseUIWidget;

	bool bHasAddToViewportCalled = false;

	UPROPERTY(EditDefaultsOnly, NoClear)
	TSubclassOf<UAccelByteWarsBaseUI> BaseUIMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSlateSound DefaultButtonClickSound;

	UPROPERTY(EditAnywhere)
	UGlobalSettingsDataAsset* GlobalSettingsDataAsset;

	UPROPERTY(EditAnywhere)
	UDataTable* GameModeDataTable;

	UPROPERTY(EditAnywhere)
	UDataTable* GameStatsDataTable;

#pragma region "AccelByte SDK Config Menu"
public:
	// Open AccelByte SDK config menu to reconfigure the config on the fly.
	UFUNCTION(Exec, BlueprintCallable)
	void OpenSDKConfigMenu();

	// Get input action data to open AccelByte SDK config menu.
	UFUNCTION(BlueprintPure)
	FDataTableRowHandle GetOpenSDKConfigMenuInputAction() { return OpenSDKConfigInputActionData; }

protected:
	// Widget class that represent AccelByte SDK config menu.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> SDKConfigWidgetClass;

	// Input action to open widget to open AccelByte SDK config menu.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataTableRowHandle OpenSDKConfigInputActionData;

private:
	// Helper variable to reference the initiated AccelByte SDK config menu.
	UAccelByteWarsActivatableWidget* SDKConfigWidgetInstance;
#pragma endregion

#pragma region "FTUE Widget"
public:
	TSubclassOf<UAccelByteWarsActivatableWidget> GetFTUEWidgetClass() const { return FTUEWidgetClass; }

	TSubclassOf<UCommonButtonStyle> GetDevHelpButtonStyle() const { return DevHelpButtonStyle; }

	FDataTableRowHandle GetDevHelpButtonInputAction() const { return DevHelpInputActionData; }

	// Open FTUE menu when in game state.
	UFUNCTION(Exec, BlueprintCallable)
	void OpenFTUEMenu();

protected:
	// Widget class that represent AccelByte FTUE Widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> FTUEWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCommonButtonStyle> DevHelpButtonStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataTableRowHandle DevHelpInputActionData;
#pragma endregion 
};
