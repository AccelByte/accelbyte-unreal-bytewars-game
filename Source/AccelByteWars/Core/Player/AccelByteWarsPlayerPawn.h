// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/GameStates/AccelByteWarsInGameGameState.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissile.h"
#include "AccelByteWars/Core/Actor/AccelByteWarsMissileTrail.h"
#include "AccelByteWars/Core/Components/AccelByteWarsProceduralMeshComponent.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerState.h"
#include "AccelByteWars/Core/Player/AccelByteWarsPlayerController.h"
#include "AccelByteWars/Core/Components/AccelByteWarsGameplayObjectComponent.h"

#include "GameFramework/RotatingMovementComponent.h"

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AccelByteWarsPlayerPawn.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAccelByteWarsPlayerPawn();

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	//~End of UObject overridden functions

public:	
	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	//~End of UObject overridden functions

	/**
	 * @brief Visible mesh of player ship
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsProceduralMeshComponent* AccelByteWarsProceduralMesh = nullptr;

	/**
	 * @brief Visible mesh of player ship
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
	 * @brief Shoots a missile
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
		void Server_FireMissile();

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
		void Server_AdjustFirePower(int Rate);

	/**
	 * @brief Lets other players know when a player ship is changing their ship color on spawn
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
		void Server_SetColor(FLinearColor InColor);

	/**
	 * @brief Event when the player ship is destroyed
	 */
	UFUNCTION(Client, Reliable, Category = AccelByteWars)
		void Client_OnDestroyed();

protected:

	/**
	 * @brief Current ship color, set only on spawn
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
		FLinearColor Color = FLinearColor::Yellow;

	/**
	 * @brief Spawns a missile with provided data
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		AAccelByteWarsMissile* SpawnMissileInWorld(AActor* ActorOwner, FTransform InTransform, float InitialSpeed, FString BlueprintPath, bool ShouldReplicate);

	/**
	 * @brief Spawns a missile trail with provided data
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		AAccelByteWarsMissileTrail* SpawnMissileTrailInWorld(AActor* ActorOwner, FTransform InTransform, FString BlueprintPath, bool ShouldReplicate);

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
