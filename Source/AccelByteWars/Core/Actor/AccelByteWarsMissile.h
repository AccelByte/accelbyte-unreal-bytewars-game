// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/Components/AccelByteWarsProceduralMeshComponent.h"
#include "AccelByteWars/Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "AccelByteWarsMissile.generated.h"

class AAccelByteWarsGameMode;

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsMissile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAccelByteWarsMissile();

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	//~End of UObject overridden functions

public:	
	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	//~End of UObject overridden functions

	/**
	 * @brief Collision values for missile
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UStaticMeshComponent* MissileMeshComponenet = nullptr;

	/**
	 * @brief Allows missile to play sounds
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAudioComponent* MissileAudioComponent = nullptr;

	/**
	 * @brief FX for missile trail
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UNiagaraComponent* ThrustSparks = nullptr;

	/**
	 * @brief FX for missile trail
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UNiagaraComponent* ExpiringSparks = nullptr;

	/**
	 * @brief FX for missile body
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsProceduralMeshComponent* AccelByteWarsProceduralMesh = nullptr;

	/**
	 * @brief Reference to generic AB physics body
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsGameplayObjectComponent* AccelByteWarsGameplayObjectComponent = nullptr;

	/**
	 * @brief Reference to generic AB physics body that has been hit
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		UAccelByteWarsGameplayObjectComponent* HitObject = nullptr;

	/**
	 * @brief Current missile velocity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Velocity)
		FVector Velocity = FVector::ZeroVector;

	/**
	 * @brief Current missile gravity force being applied from planets
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = AccelByteWars)
		FVector GravityForce = FVector::ZeroVector;

	/**
	 * @brief Constant used in calculating gravity forces
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float GravitationalConstant = 1.0f;

	/**
	 * @brief Indicates if the missile should be destroyed this frame
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		bool KillActorThisFrame = false;

	/**
	 * @brief Max time the missile is allowed to be alive if it doesn't hit anything
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float MaxTimeAlive = 20.0f;

	/**
	 * @brief Current score
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float Score = 1000.0f;

	/**
	 * @brief Current score increment
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float ScoreIncrement = 100.0f;

	/**
	 * @brief Amount of time the missile is skimming a planet (for added score)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float TimeSkimmingPlanet = 0.0f;

	/**
	 * @brief Reward calculation for skimming planet
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float TimeSkimmingPlanetReward = 0.0f;

	/**
	 * @brief Allowed time between giving score rewards when skimming
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float TimeBetweenRewards = 0.25f;

	/**
	 * @brief When the score time expires and no more points are given
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float ExpiryTime = 4.0f;

	/**
	 * @brief Has the skimming points stopped accumulating for the current skim?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		bool Expiring = false;

	/**
	 * @brief Current time the missile has been alive
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float TimeAlive = 0.0f;

	/**
	 * @brief Spawning speed of the missile
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float InitialSpeed = 0.0f;

	/**
	 * @brief Saves the current DeltaTime from tick to be used elsewhere
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		float TickDeltaSeconds = 0.0f;

	/**
	 * @brief Current ships that the missile is zooming past (for skimming)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
		TArray<AActor*> NearHitShips;

	/**
	 * @brief Destroy the missile of owner is destroyed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
		void DestroyActorOnOwnerDestroyed();

	/**
	 * @brief Initially sets the velocity when fired
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void SetVelocity();

	/**
	 * @brief Does the spawning of the score popups when the player missile does something score worthy
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
		void SpawnScorePopupHud(int InScore);

	/**
	 * @brief Starts the destruction of the missile for various reasons
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = AccelByteWars)
		void TreatMissileAsExpired(AAccelByteWarsGameMode* ABGameMode);

	/**
	 * @brief Removes the count of the current missiles when one is destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void DecrementPlayerMissileCount();

	/**
	 * @brief Returns true if the missile is skimming a planet
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		bool IsSkimmingPlanet(TArray<UAccelByteWarsGameplayObjectComponent*> ABObjectComponent, UAccelByteWarsGameplayObjectComponent* ABGameplayObject);

	/**
	 * @brief Returns true if the missile is skimming a player ship
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		bool IsNearHitShip(UAccelByteWarsGameplayObjectComponent* ABObjectComponent);

	/**
	 * @brief Calculates the current gravity force to any given planet
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void GetGravityForceToObject(UAccelByteWarsGameplayObjectComponent* OtherObject, UAccelByteWarsGameplayObjectComponent* ThisObject, float& RetValue1, FVector& RetValue2);

	/**
	 * @brief Points the missile towards the direction it is currently traveling
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void AlignWithVelocityDirection(FVector InVector);

	/**
	 * @brief Lets everyone in the match know what color the missile is
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
		void Server_SetColor(FLinearColor InColor);

	/**
	 * @brief Calculates gravity applied to game objects
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void ApplyGravityToThisGameObjects();

	/**
	 * @brief Applies gravity forces to velocity
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void ApplyOverallGravityForceToChangeTheVelocity(float DeltaTime);

	/**
	 * @brief Inform server of forward vector changes
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
		void Server_SetMissileForwardVector(FVector DeltaAdjustedVelocity, FVector NewVelocity);

	/**
	 * @brief Activates expiring animation for missile at end of life
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void ExpiryWindowBeforeTimeoutDestruction();

	/**
	 * @brief Initiates end of life sequence for missile
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void DestroyOnTimeout();

	/**
	 * @brief Initiates end of life sequence for missile when a Byte Bomb is used
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void DestroyByPowerUp();

	/**
	 * @brief Updates score depending on time and skimming
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void SkimmingAndScoreUpdate(float DeltaTime);

	/**
	 * @brief Handle when missile strikes a ship
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void OnDestroyObject();

	/**
	 * @brief Initiates end of life sequence for missile when it goes out of bounds
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
		void DestroyOnOutOfBounds();


protected:

	/**
	 * @brief Current color of the missile body
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
		FLinearColor Color;

	/**
	 * @brief Generic on rep notify for the missile color
	 */
	UFUNCTION()
		void OnRepNotify_Color();

	/**
	 * @brief Generic on rep notify for the Vector updates
	 */
	UFUNCTION()
		void OnRepNotify_Velocity();

	/**
	 * @brief Calculates the distance between objects in 2D space
	 */
	UFUNCTION()
		float GetSurfanceDistanceBetweenObjects(UAccelByteWarsGameplayObjectComponent* OtherObject, UAccelByteWarsGameplayObjectComponent* ThisObject);
};
