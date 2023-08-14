// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "AccelByteWarsInGameGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPlayerDieDelegate, const APlayerController* /*Player*/, const AActor* /*PlayerActor*/, const APlayerController* /*Killer*/);

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsInGameGameMode : public AAccelByteWarsGameMode
{
	GENERATED_BODY()

public:
	static inline FSimpleMulticastDelegate OnGameEndsDelegate;
	static inline FOnPlayerDieDelegate OnPlayerDieDelegate;

	//~AGameModeBase overridden functions
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
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

	UFUNCTION(BlueprintCallable)
	void EndGame(const FString Reason = "");
	
	UFUNCTION(BlueprintCallable)
	void OnShipDestroyed(
		UAccelByteWarsGameplayObjectComponent* Ship,
		const float MissileScore,
		APlayerController* SourcePlayerController);

	/**
	 * @brief Increase the number of attempt the player was almost got killed in a single-lifetime.
	 * @param TargetPlayer The player that was almost got killed.
	 */
	UFUNCTION(BlueprintCallable)
	void IncreasePlayerKilledAttempt(const APlayerController* TargetPlayer);

private:
	UFUNCTION()
	void RemoveFromActiveGameObjects(AActor* DestroyedActor);

	// Gameplay logic
	void CloseGame(const FString& Reason) const;
	void StartGame();
	void SetupGameplayObject(AActor* Object) const;
	bool CheckIfAllPlayersIsInOneTeam() const;
	void SpawnAndPossesPawn(APlayerState* PlayerState);
	TArray<FVector> GetActiveGameObjectsPosition(const float SeparationFactor, const float ShipSeparationFactor) const;
	FVector FindGoodPlanetPosition() const;
	FVector FindGoodPlayerPosition() const;

	// Countdown related
	void NotEnoughPlayerCountdownCounting(const float& DeltaSeconds) const;
	void SetupShutdownCountdownsValue() const;

	// Gameplay logic math helper
	/**
	 * @brief Calculate random location within game's bounding area outside all active game objects radius.
	 * If bounding area < total game objects radius, then increase game's bounding area.
	 */
	FVector2D FindGoodSpawnLocation(const TArray<FVector>& ActiveGameObjectsCoords) const;
	bool IsInsideCircle(const FVector2D& Target, const FVector& Circle) const;
	double CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord) const;
	FVector2D CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX) const;
	bool LocationHasLineOfSightToOtherShip(const FVector& PositionToTest) const;

	const TArray<FVector> PlayerStartPoints = {
		{400.0f, 500.0f, 0.0f},
		{-400.0f, -500.0f, 0.0f},
		{0.0f, -500.0f, 0.0f},
		{-400.0f, 500.0f, 0.0f}
	};

	float GameEndsDelay = 1.0f;

	UPROPERTY()
	AAccelByteWarsInGameGameState* ABInGameGameState = nullptr;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	APawn* CreatePlayerPawn(const FVector& Location, const FLinearColor& Color, APlayerController* PlayerController);

	UFUNCTION(BlueprintImplementableEvent)
	void OnShipDestroyedFX(
		APlayerController* SourcePlayerController,
		const FTransform ShipTransform,
		AAccelByteWarsPlayerState* ShipPlayerState);

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ObjectsToSpawn;
};
