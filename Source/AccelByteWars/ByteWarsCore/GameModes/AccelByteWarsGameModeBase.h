// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsGameStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_CLASS(LogByteWarsGameMode, Log, All);

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	//~AGameModeBase overridden functions
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//~End of AGameModeBase overridden functions

	/**
	 * @brief Add player's score in GameState and PlayerState
	 * @param PlayerState Target PlayerState
	 * @param InScore Value to be added to player's score
	 * @param bAddKillCount Should increase player's kill count
	 * @return Player's new score
	 */
	UFUNCTION(BlueprintCallable) UPARAM(DisplayName = "CurrentScore")
	int32 AddPlayerScore(APlayerState* PlayerState, float InScore, bool bAddKillCount = true);

	/**
	 * @brief 
	 * @param PlayerState Target PlayerState
	 * @param Decrement Value to be subtracted from player's life count
	 * @return Player's new life count
	 */
	UFUNCTION(BlueprintCallable) UPARAM(DisplayName = "CurrentLifes")
	int32 DecreasePlayerLife(APlayerState* PlayerState, uint8 Decrement = 1);

	/**
	 * @brief Reset stored game data in GameState
	 */
	UFUNCTION(BlueprintCallable)
	void ResetGameData();

	UFUNCTION(BlueprintCallable)
	void TriggerServerTravel(TSoftObjectPtr<UWorld> Level);

protected:

	UFUNCTION(BlueprintCallable)
	void PlayerSetup(APlayerController* PlayerController) const;

	UPROPERTY(EditAnywhere)
	bool bIsGameplayLevel = false;

private:

	UPROPERTY()
	AAccelByteWarsGameStateBase* ByteWarsGameState = nullptr;

	UPROPERTY()
	UAccelByteWarsGameInstance* ByteWarsGameInstance = nullptr;

	static FUniqueNetIdRepl GetPlayerUniqueNetId(const APlayerController* PlayerController);

	static int32 GetControllerId(const APlayerState* PlayerState);
};
