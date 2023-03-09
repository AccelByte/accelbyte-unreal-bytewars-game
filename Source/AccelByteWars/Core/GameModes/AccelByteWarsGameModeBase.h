// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsGameStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameModeBase.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsGameMode, Log, All);

#define GAMEMODE_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsGameMode, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

DECLARE_DELEGATE_RetVal_OneParam(int32, FOnGetTeamIdFromSession, APlayerController* /*PlayerController*/)

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	AAccelByteWarsGameModeBase();

	//~AGameModeBase overridden functions
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	//~End of AGameModeBase overridden functions

public:

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

	/**
	 * @brief Executed after Pre-game countdown finished
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartGame();

	inline static FOnGetTeamIdFromSession OnGetTeamIdFromSessionDelegate;

protected:

	UFUNCTION(BlueprintCallable)
	void PlayerTeamSetup(APlayerController* PlayerController) const;

	/**
	 * @brief Add player to specific team. Use this instead of AAccelByteWarsGameStateBase::AddPlayerToTeam due to how
	 * UE handle UniqueNetId in Blueprint.
	 * @param PlayerController PlayerController to be added
	 * @param TeamId Target TeamId, if doesn't exist, it'll be created
	 */
	UFUNCTION(BlueprintCallable)
	void AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId);

	bool RemovePlayer(const APlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable)
	void EndGame(const FString Reason = "");

	UPROPERTY(EditAnywhere)
	bool bIsGameplayLevel = false;

	UPROPERTY(EditAnywhere)
	bool bShouldRemovePlayerOnLogoutImmediately = false;

	UPROPERTY(BlueprintReadOnly)
	float GameEndedTime = 0.0f;

private:

	void AssignTeamManually(int32& InOutTeamId) const;

	UPROPERTY()
	AAccelByteWarsGameStateBase* ByteWarsGameState = nullptr;

	UPROPERTY()
	UAccelByteWarsGameInstance* ByteWarsGameInstance = nullptr;

	static FUniqueNetIdRepl GetPlayerUniqueNetId(const APlayerController* PlayerController);

	static int32 GetControllerId(const APlayerState* PlayerState);

	bool CheckIfAllPlayersIsInOneTeam() const;
	void SetupShutdownCountdownsValue() const;
	void CloseGame(const FString& Reason) const;
};
