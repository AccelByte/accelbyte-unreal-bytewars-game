// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameMode.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsGameMode, Log, All);

#define GAMEMODE_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsGameMode, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

class AAccelByteWarsPlayerState;

#pragma region "Structs, Enums, and Delegates declaration"
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerPostLogin, APlayerController* /*NewPlayer*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInitializeListenServer, FName /* SessionName */);
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

// @@@SNIPSTART AccelByteWarsGameMode.h-public
// @@@MULTISNIP CloseGame {"selectedLines": ["1", "34"]}
// @@@MULTISNIP OnPreGameShutdownDelegate {"selectedLines": ["1", "42"]}
public:
	AAccelByteWarsGameMode();

	//~AGameModeBase overridden functions
	
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	//~End of AGameModeBase overridden functions

	/**
	 * @brief Reset stored game data in GameState
	 */
	UFUNCTION(BlueprintCallable)
	void ResetGameData();

	UFUNCTION(BlueprintCallable)
	void PlayerTeamSetup(APlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable)
	virtual void DelayedPlayerTeamSetupWithPredefinedData(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	void AssignTeamManually(int32& InOutTeamId) const;

	UFUNCTION(BlueprintCallable)
	void KickAllPlayers() const;

	UFUNCTION(BlueprintCallable)
	void ServerTravel(TSoftObjectPtr<UWorld> Level);

	void DelayedServerTravel(const FString& URL) const;

	/** @brief Shutdown DS and call unregister server if possible */
	void CloseGame(const FString& Reason);

	/** @brief Set the flag to allow immediate shutdown when the last player logs out */
	void SetImmediatelyShutdownWhenEmpty(const bool bAllow) const;

	inline static FOnPlayerPostLogin OnPlayerPostLoginDelegates;
	inline static FOnInitializeListenServer OnInitializeListenServerDelegates;
	inline static TMulticastDelegate<void(bool /*bSucceeded*/)> OnRegisterServerCompleteDelegates;
	static inline TMulticastDelegate<void(TDelegate<void()>)> OnPreGameShutdown;
// @@@SNIPEND

protected:
	/**
	 * @brief Add player to specific team. Use this instead of AAccelByteWarsGameStateBase::AddPlayerToTeam due to how
	 * UE handle UniqueNetId in Blueprint.
	 * @param PlayerController PlayerController to be added
	 * @param TeamId Target TeamId, if doesn't exist, it'll be created
	 */
	UFUNCTION(BlueprintCallable)
	void AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId);

	bool RemovePlayer(const APlayerController* PlayerController) const;

	UPROPERTY(EditDefaultsOnly)
	bool bIsGameplayLevel = false;

	UPROPERTY(EditDefaultsOnly)
	bool bShouldRemovePlayerOnLogoutImmediately = false;

	bool IsServer() const;

	// Countdown functionalities to close the server.
	void SetupServerCloseCountdownValue();
	void ServerCloseCountdownCounting(const float& DeltaSeconds);

	// Countdown functionalities to simulate server crash.
	void SetupSimulateServerCrashCountdownValue(const FString& SimulateServerCrashArg);
	void SimulateServerCrashCountdownCounting(const float& DeltaSeconds) const;

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance = nullptr;

private:
	UFUNCTION()
	void CloseGameInternal() const;

	UPROPERTY()
	AAccelByteWarsGameState* ABGameState = nullptr;

	bool bIsServerClosing = false;

	/* Helper variable to check whether server should simulate 
	 * crash or not based on the availability of launch param.*/
	bool bShouldSimulateServerCrash = false;

	mutable bool bImmediatelyShutdownWhenEmpty = false;
};
