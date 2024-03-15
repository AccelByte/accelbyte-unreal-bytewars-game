// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Engine/SCS_Node.h"
#include "AccelByteWarsInGameGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPlayerDieDelegate, const APlayerController* /*Player*/, const AActor* /*PlayerActor*/, const APlayerController* /*Killer*/);

USTRUCT(BlueprintType)
struct FPlanetMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlanetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlanetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlanetRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D PlanetPosition;

	FPlanetMetadata()
		: PlanetID(0), PlanetName(TEXT("")), PlanetRadius(0.0f), PlanetPosition(FVector2D::ZeroVector)
	{
	}

	FPlanetMetadata(int32 InPlanetID, FString InPlanetName, float InPlanetRadius, FVector2D InPlanetPosition = FVector2D::ZeroVector)
		: PlanetID(InPlanetID), PlanetName(InPlanetName), PlanetRadius(InPlanetRadius), PlanetPosition(InPlanetPosition)
	{
	}
	
	FPlanetMetadata& operator=(const FPlanetMetadata& Other)
	{
		PlanetID = Other.PlanetID;
		PlanetName = Other.PlanetName;
		PlanetRadius = Other.PlanetRadius;
		return *this;
	}
};

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsInGameGameMode : public AAccelByteWarsGameMode
{
	GENERATED_BODY()

public:
	AAccelByteWarsInGameGameMode();

	static inline FSimpleMulticastDelegate OnGameEndsDelegate;
	static inline FOnPlayerDieDelegate OnPlayerDieDelegate;

#pragma region "Variables"
private:
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
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ObjectsToSpawn;

	// Can't retrieve objects` actor component before initiating it, use this preset instead
	const TMap<int32, FPlanetMetadata> PlanetMap = {
		{0, FPlanetMetadata{ 0, TEXT("SmallRed"), 100.0f }},
		{1, FPlanetMetadata{ 1, TEXT("SmallRed2"), 125.0f }},
		{2, FPlanetMetadata{ 2, TEXT("MediumOrange"), 150.0f }},
		{3, FPlanetMetadata{ 3, TEXT("MediumOrange2"), 175.0f }},
		{4, FPlanetMetadata{ 4, TEXT("LargeBlue"), 200.0f }},
		{5, FPlanetMetadata{ 5, TEXT("LargeBlue2"), 225.0f }}
	};

public:
	// Zone in which players' can't be spawn along the x and y axis of the the center of the play area 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float SafeZone = 150.0f;

	// Maximum number of planets to be spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawn Settings")
	int32 MaxTargetPlanetCount = 5;

	// Planets' spawn area percentage from the center of the map. 100.0f means all of the play area
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawn Settings")
	float PlanetSpawnAreaPercentage = 70.0f;

	// gap between objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawn Settings")
	float ObjectSafeDistance = 400.0f;
#pragma endregion

	//~AGameModeBase overridden functions
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//~End of AGameModeBase overridden functions

	/**
	 * @brief Direct path to ABPawn to be spawned
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
		FString PawnBlueprintPath = "Blueprint'/Game/ByteWars/Blueprints/Player/ABPawn.ABPawn_C'";

	virtual void DelayedPlayerTeamSetupWithPredefinedData(APlayerController* PlayerController) override;

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

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
	void OnMissileDestroyed(FVector Location, UAccelByteWarsGameplayObjectComponent* HitObject, FLinearColor MissileColour, AActor* MissileOwner);

	/**
	 * @brief Increase the number of attempt the player was almost got killed in a single-lifetime.
	 * @param TargetPlayer The player that was almost got killed.
	 */
	UFUNCTION(BlueprintCallable)
	void IncreasePlayerKilledAttempt(const APlayerController* TargetPlayer);

	UFUNCTION(BlueprintCallable)
	void OnRefreshPlayerSelectedPowerUp(const APlayerController* TargetPlayer, const EPowerUpSelection SelectedPowerUp, const int32 PowerUpCount);

private:
	UFUNCTION()
	void RemoveFromActiveGameObjects(AActor* DestroyedActor);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShipDestroyedFX(
		APlayerController* SourcePlayerController,
		const FTransform ShipTransform,
		AAccelByteWarsPlayerState* ShipPlayerState);

#pragma region "Gameplay logic"
private:
	void CloseGame(const FString& Reason) const;
	void StartGame();
	void SetupGameplayObject(AActor* Object) const;
	int32 GetLivingTeamCount() const;
	void SpawnAndPossesPawn(APlayerState* PlayerState);

	TArray<FVector> GetActiveGameObjectsPosition() const;
public:
	/**
	 * @brief Find randomize spawn location that is not occupied with other object and within the gameplay 
	 */
	bool FindGoodSpawnLocation(FVector2D& OutCoord);
private:
	void SpawnPlanets();
	bool FindGoodPlanetPosition(FVector& Position) const;
	
	// #jog afif Need to replace team id with player id
	FVector FindGoodPlayerPosition(APlayerState* PlayerState) const;

protected:
	UFUNCTION()
	APawn* CreatePlayerPawn(const FVector& Location, APlayerController* PlayerController);
#pragma endregion 

#pragma region "Countdown related"
private:
	bool ShouldStartNotEnoughPlayerCountdown() const;
	void NotEnoughPlayerCountdownCounting(const float& DeltaSeconds) const;
	void SetupShutdownCountdownsValue() const;
#pragma endregion 

#pragma region "Gameplay logic math helper"
private:
	/**
	 * @brief Pseudorandom coordinate with a rectangle bounding box
	 * @param OutCoord Calculated coord
	 * @param ActiveGameObjectsCoords List of existing object coords that will be included in the pseudorandom algorithm
	 * @param MinBound Spawn area min bound
	 * @param MaxBound Spawn area max bound
	 * @return true if good location found, false if area is too cramped
	 */
	bool FindGoodSpawnLocation(
		FVector2D& OutCoord,
		const TArray<FVector>& ActiveGameObjectsCoords,
		const FVector2D& MinBound,
		const FVector2D& MaxBound) const;

	bool IsInsideCircle(const FVector2D& Target, const FVector& Circle) const;
	double CalculateCircleLengthAlongXonY(const FVector& Circle, const double Ycoord) const;
	FVector2D CalculateActualCoord(const double RelativeLocation, const FVector2D& MinBound, const double RangeX) const;
	bool LocationHasLineOfSightToOtherShip(const FVector& PositionToTest) const;
#pragma endregion 

#pragma region "Debugging"
	UFUNCTION(Exec)
	void StartPlanetSpawningTimer(float InRate);

	UFUNCTION(Exec)
	void StopPlanetSpawningTimer();
	
	UFUNCTION(Exec)
	void ResetPlanets();

	UFUNCTION(Exec)
	void SetPlanetTargetNum(const int32 NewTarget);

	UFUNCTION(Exec)
	void SetObjectSafeDistance(float NewDistance);

	UFUNCTION(Exec)
	void DrawBoundingBoxOnNextSpawn(const bool bDraw);

	UFUNCTION(Exec)
	void ModifyGameBoundExtendModifier(const float NewModifier) const;

protected:
	UPROPERTY(EditAnywhere)
	bool bDrawBoundingBox = false;

private:
	FTimerHandle PlanetSpawningTimerHandle;
#pragma endregion
};
