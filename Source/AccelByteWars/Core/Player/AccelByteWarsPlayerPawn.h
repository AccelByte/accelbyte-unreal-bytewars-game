// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

// Player Ships
#include "AccelByteWars/Core/Ships/PlayerShipBase.h"
#include "AccelByteWars/Core/Ships/PlayerShipTriangle.h"
#include "AccelByteWars/Core/Ships/PlayerShipD.h"
#include "AccelByteWars/Core/Ships/PlayerShipDoubleTriangle.h"
#include "AccelByteWars/Core/Ships/PlayerShipGlowXtra.h"
#include "AccelByteWars/Core/Ships/PlayerShipWhiteStar.h"

// Power Ups
#include "AccelByteWars/Core/PowerUps/PowerUpBase.h"
#include "AccelByteWars/Core/PowerUps/PowerUpByteBomb.h"
#include "AccelByteWars/Core/PowerUps/PowerUpByteShield.h"
#include "AccelByteWars/Core/PowerUps/PowerUpWormHole.h"
#include "AccelByteWars/Core/PowerUps/PowerUpSplitMissile.h"

#include "AccelByteWars/Core/GameStates/AccelByteWarsInGameGameState.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissileTrail.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerState.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerController.h"
#include "AccelByteWars/Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "AccelByteWars/Core/UI/InGameMenu/HUD/HUDPlayer.h"

#include "GameFramework/RotatingMovementComponent.h"
#include "Components/SphereComponent.h"

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AccelByteWarsPlayerPawn.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnMatchStarted, AAccelByteWarsPlayerPawn* /*PlayerPawn*/, const APlayerController* /*PlayerController*/, const FLinearColor /*InColor*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPlayerEquipmentLoaded, AAccelByteWarsPlayerPawn* /*PlayerPawn*/, const EShipDesign /*SelectedShipDesign*/, const EPowerUpSelection /*SelectedPowerUp*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnValidateActivatePowerUp, AAccelByteWarsPlayerPawn* /*PlayerPawn*/, const EPowerUpSelection /*SelectedPowerUp*/);
 
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	static inline FOnMatchStarted OnMatchStarted;
	static inline FOnPlayerEquipmentLoaded OnPlayerEquipmentLoaded;
	static inline FOnValidateActivatePowerUp OnValidateActivatePowerUp;

	// Sets default values for this pawn's properties
	AAccelByteWarsPlayerPawn();

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	//~End of UObject overridden functions

public:	
	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UObject overridden functions

	/**
	 * @brief A do nothing root component of player ship
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
	class USphereComponent* SphereComponent = nullptr;

	/**
	 * @brief Player ship look and feel class
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
	class APlayerShipBase* PlayerShip = nullptr;

	/**
	 * @brief Currently equipped power up
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
	class APowerUpBase* PowerUp = nullptr;

	/**
	 * @brief For gravity calculations
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UAccelByteWarsGameplayObjectComponent* GameplayObject = nullptr;

	/**
	 * @brief Referenced to latest fired missile
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	AAccelByteWarsMissile* FiredMissile = nullptr;

	/**
	 * @brief Referenced to latest fired missile
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	AAccelByteWarsMissileTrail* MissileTrail = nullptr;

	/**
	 * @brief Direct path to generic ABPlayerShip to be spawned
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	TArray<FString> PlayerShipBlueprintPaths = 
	{
		"Blueprint'/Game/ByteWars/Blueprints/Ships/ABPlayerShipTriangle.ABPlayerShipTriangle_C'",
		"Blueprint'/Game/ByteWars/Blueprints/Ships/ABPlayerShipD.ABPlayerShipD_C'",
		"Blueprint'/Game/ByteWars/Blueprints/Ships/ABPlayerShipDoubleTriangle.ABPlayerShipDoubleTriangle_C'",
		"Blueprint'/Game/ByteWars/Blueprints/Ships/ABPlayerShipGlowXtra.ABPlayerShipGlowXtra_C'",
		"Blueprint'/Game/ByteWars/Blueprints/Ships/ABPlayerShipWhiteStar.ABPlayerShipWhiteStar_C'"
	};

	/**
	 * @brief Direct path to generic ABPlayerShip to be spawned
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	TArray<FString> PlayerPowerUpBlueprintPaths = 
	{
		"", // Empty string indicates NONE PowerUp selected
		"Blueprint'/Game/ByteWars/Blueprints/PowerUps/ABByteBomb.ABByteBomb_C'",
		"Blueprint'/Game/ByteWars/Blueprints/PowerUps/ABByteShield.ABByteShield_C'",
		"Blueprint'/Game/ByteWars/Blueprints/PowerUps/ABWormHole.ABWormHole_C'",
		"Blueprint'/Game/ByteWars/Blueprints/PowerUps/ABSplitMissile.ABSplitMissile_C'",
	};

	/**
	 * @brief Direct path to ABMissile to be spawned
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	FString FiredMissileBlueprintPath = "Blueprint'/Game/ByteWars/Blueprints/Missiles/ABMissile.ABMissile_C'";

	/**
	 * @brief Direct path to ABMissileTrail to be spawned
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	FString FiredMissileTrailBlueprintPath = "Blueprint'/Game/ByteWars/Blueprints/Missiles/ABMissileTrail.ABMissileTrail_C'";

	/**
	 * @brief Current power level of the missile about to be fired
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_FirePowerLevel)
	float FirePowerLevel = 0.5f;

	/**
	 * @brief Used for calculating fire power
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float FirePowerAdjustRate = 0.0f;

	/**
	 * @brief Used for calculating fire power
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	float ConstFirePowerAdjustRate = 0.5f;

	/**
	 * @brief Constant for turning the player ship
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	float ConstRotateRate = 2.0f;

	/**
	 * @brief How long the power level indicator should be shown on screen
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float ShowPowerLevelUITimer = 0.0f;

	/**
	 * @brief How long the ship label indicator should be shown on screen
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	float ShowShipLabelUITimer = 5.0f;

	/**
	 * @brief Minimum allowed missile speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float MinMissileSpeed = 150.0f;

	/**
	 * @brief Maximum allowed missile speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float MaxMissileSpeed = 900.0f;

	/**
	 * @brief Lets the pawn know that it has been destroyed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	bool IsDestroyed = false;

	/**
	 * @brief True if the player's ship is currently rotating
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_RotationDirection)
	int RotationDirection = 0;

	/**
	 * @brief Player ship direction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_CurrentYaw)
	float CurrentYaw = 0.0f;

	/**
	 * @brief Plays the sounds for shooting a missile
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void FireMissileFx(FTransform SpawnTransform);

	/**
	 * @brief Calculates the transform to be passed to the missile and missile trail when firing a missile
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	FTransform CalculateWhereToSpawnMissile();

	/**
	 * @brief Visually updates the power bar UI
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void UpdatePowerBarUI(float DeltaTime);

	/**
	 * @brief Visually updates the ship label UI
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void UpdateShipLabelUI(float DeltaTime);

	/**
	 * @brief Updated when the player sends some input
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void OnPlayerInputThisFrame();

	/**
	 * @brief Fires off the Game Camera Pulse Background Event
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
	void PulseCameraBackground();

	/**
	 * @brief Fires off the Game Camera Pulse Background Event
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void PauseCameraBackgroud();

	/**
	 * @brief Fires off the Game Camera Pulse Background Event
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	UAccelByteWarsProceduralMeshComponent* GetPlayerShipMesh();

	// ***************** //
	// *** RPC Start *** //
	// ***************** //

	/**
	 * @brief Gets players selected ship
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_GetPlayerSelectedShip(APlayerController* PlayerController, FLinearColor InColor);

	/**
	 * @brief Gets players selected ship
	 */
	UFUNCTION(BlueprintCallable, Client, Reliable, Category = AccelByteWars)
	void Client_GetPlayerSelectedShip(APlayerController* PlayerController, FLinearColor InColor);

	/**
	 * @brief Visually creates player ship
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_SpawnPlayerShip(const EShipDesign SelectedShipDesign);

	/**
	 * @brief Validate and activate power up (client side).
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void ValidateActivatePowerUp(const EPowerUpSelection SelectedPowerUp);

	/**
	 * @brief Refresh game client power up state (server side).
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_RefreshSelectedPowerUp(const EPowerUpSelection SelectedPowerUp, const int32 PowerUpCount);

	/**
	 * @brief Activate power up (server side).
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_ActivatePowerUp(const EPowerUpSelection SelectedPowerUp);

	/**
	 * @brief Shoots a missile
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_FireMissile();

	/**
	 * @brief Lets other players know when a player ship is rotating
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_RotatePawn(int Rate);

	/**
	 * @brief Informs the local client they are turning
	 */
	UFUNCTION(BlueprintCallable, Client, Reliable, Category = AccelByteWars)
	void Client_RotatePawn(int Rate);

	/**
	 * @brief Lets other players know when a player ship is changing their attack power
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_AdjustFirePower(FVector PlayerPosition, int Rate);

	/**
	 * @brief Client sends to update power bar on the UI
	 */
	UFUNCTION(BlueprintCallable, Client, Reliable, Category = AccelByteWars)
	void Client_AdjustFirePower(FVector PlayerPosition, FLinearColor InColor);

	/**
	 * @brief Lets other players know when a player ship is changing their ship color on spawn
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_SetColor(FLinearColor InColor);

	/**
	 * @brief Event when the player ship is destroyed
	 */
	UFUNCTION(Client, Reliable, Category = AccelByteWars)
	void Client_OnDestroyed();

	// ***************** //
	// **** RPC End **** //
	// ***************** //

protected:

	/**
	 * @brief Current ship color, set only on spawn
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
	FLinearColor PawnColor = FLinearColor::Yellow;

	/**
	 * @brief Spawns a missile with provided data
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	AAccelByteWarsMissile* SpawnMissileInWorld(AActor* ActorOwner, FTransform InTransform, float InitialSpeed, FString BlueprintPath, bool ShouldReplicate);

	template<class T>
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	T* SpawnBPActorInWorld(APawn* OwningPawn, const FVector Location, const FRotator Rotation, FString BlueprintPath, bool ShouldReplicate);

	/**
	 * @brief Determines if the player's ship is able to fire a missile
	 */
	bool ShouldFire();

	/**
	 * @brief Takes in -1, 0, or 1 to adjust the fire power level
	 */
	void AdjustFirePower(int Rate);

	/**
	 * @brief Generic OnRep notify for color update
	 */
	UFUNCTION()
	void OnRepNotify_Color();

	/**
	 * @brief Generic OnRep notify for player ship power level
	 */
	UFUNCTION()
	void OnRepNotify_FirePowerLevel();

	/**
	 * @brief Generic OnRep notify for player rotation flag
	 */
	UFUNCTION()
	void OnRepNotify_RotationDirection();

	/**
	 * @brief Generic OnRep notify for player ship rotation
	 */
	UFUNCTION()
	void OnRepNotify_CurrentYaw();

};
