// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsGameStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_CLASS(LogByteWarsGameMode, Log, All);

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	//~AGameModeBase interface
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	//~End of AGameModeBase interface

	UFUNCTION(BlueprintCallable) UPARAM(DisplayName = "CurrentScore")
	int32 AddPlayerScore(APlayerState* PlayerState, float InScore, bool bAddKillCount = true);

	UFUNCTION(BlueprintCallable) UPARAM(DisplayName = "CurrentLifes")
	int32 DecreasePlayerLife(APlayerState* PlayerState, uint8 Decrement = 1);

#pragma region "GameSetup related"
	UFUNCTION(BlueprintCallable)
	void AssignGameMode(FString CodeName) const;

protected:
	FGameModeData GetGameModeDataByCodeName(const FString CodeName) const;

	UFUNCTION(BlueprintCallable)
	void PlayerSetup(APlayerController* PlayerController) const;

	UPROPERTY(EditAnywhere)
	bool bIsGameplayLevel = false;

	UPROPERTY(EditAnywhere)
	UDataTable* GameModeDataTable;
#pragma endregion

private:

	UPROPERTY()
	AAccelByteWarsGameStateBase* AccelByteWarsGameState = nullptr;

	UPROPERTY()
	UAccelByteWarsGameInstance* AccelByteWarsGameInstance = nullptr;

	static FUniqueNetIdRepl GetPlayerUniqueNetId(const APlayerController* PlayerController);
};
