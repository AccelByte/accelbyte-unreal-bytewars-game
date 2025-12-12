// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AccelByteWarsPlayerPawn.generated.h"

class APlayerShipBase;
class UHUDShipLabelWidget;
class UAccelByteWarsGameplayObjectComponent;
class AAccelByteWarsMissile;
class UAccelByteWarsGameplayObjectComponent;
class UAccelByteWarsProceduralMeshComponent;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActivatePowerUp, const APlayerController* PlayerController, const FString& ItemId)

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAccelByteWarsPlayerPawn();

	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void Destroyed() override;
	//~End of UObject overridden functions

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	// Notify other object that a power up has been used
	static inline FOnPowerUpUsed OnPowerUpUsedServerDelegates;

	/**
	 * @brief A do nothing root component of player ship
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
	class USphereComponent* SphereComponent = nullptr;

	/**
	 * @brief Player ship look and feel class
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars)
	APlayerShipBase* PlayerShip = nullptr;

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

	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars, EditAnywhere)
	TSubclassOf<AActor> MissileActor;

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
	 * @brief Cooldown duration in seconds between missile fires
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float MissileCooldownDuration = 3.0f;

	/**
	 * @brief Last time a missile was fired (in world seconds)
	 */
	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars)
	float LastMissileFireTime = 0.0f;

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
	 * @brief Calculates the transform to be passed to the missile when firing a missile.
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	FTransform CalculateWhereToSpawnMissile();

	/**
	 * @brief Visually updates the power bar UI
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void UpdatePowerBarUI(float DeltaTime);

	/**
	 * @brief Visually updates the player label.
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void UpdateShipLabel();

	/**
	 * @brief Toggles the visibility of the player label.
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void ToggleShipLabel();

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
	 * @brief Pause the Game Camera Pulse Background Event
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = AccelByteWars)
	void PauseCameraBackgroud();

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void UpdateSkin();

	static inline FOnActivatePowerUp OnPowerUpActivatedDelegates;

	/** @brief Update ship glow based on currently fired missile */
	void UpdateShipGlow();

	// ***************** //
	// *** RPC Start *** //
	// ***************** //

	/** @brief Activate power up (server side). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void ServerActivatePowerUp();

	/** @brief Notify client that the power up was successfully activated */
	UFUNCTION(Client, Reliable)
	void ClientPowerUpActivated();

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
	 * @brief Event when the player ship is destroyed
	 */
	UFUNCTION(Client, Reliable, Category = AccelByteWars)
	void Client_OnDestroyed();

	// ***************** //
	// **** RPC End **** //
	// ***************** //

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	const FLinearColor GetPawnColor() const { return PawnColor; }

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	const UInGameItemDataAsset* GetSkinAsset() const { return SkinAsset; }

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	const UInGameItemDataAsset* GetColorAsset() const { return ColorAsset; }

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	const UInGameItemDataAsset* GetMissileTrailFxAsset() const { return MissileTrailFxAsset; }

	/**
	 * @brief Lets other players know when a player ship is changing their ship color on spawn
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void SetColor(FLinearColor InColor);

	/**
	 * @brief Retrieve the currently active and attached power up to this pawn actor
	 */
	IInGameItemInterface* GetActivePowerUp() const;

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	//~End of UObject overridden functions

	void InitDataAssets();

	template <class T>
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	T* SpawnActorInWorld(
		AActor* Owner,
		const FVector Location,
		const FRotator Rotation,
		TSubclassOf<AActor> ActorClass,
		bool ShouldReplicate,
		TFunction<void(AActor*)> PreSpawnInitialization = TFunction<void(AActor*)>());

	/**
	 * @brief Determines if the player's ship is able to fire a missile
	 */
	bool ShouldFire();

	/**
	 * @brief Resets the missile cooldown timer
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void ResetMissileCooldown();

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

	/**
	 * @brief Callback when a tracked missile is destroyed; removes it from tracking collection
	 */
	UFUNCTION()
	void OnTrackedMissileDestroyed(AActor* DestroyedActor);

	/**
	 * @brief Current ship color, set only on spawn
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
	FLinearColor PawnColor = FLinearColor::White;

	/**
	 * @brief Missiles fired by this pawn that are currently alive (server-side tracking)
	 */
	UPROPERTY()
	TSet<TWeakObjectPtr<AAccelByteWarsMissile>> TrackedMissiles;

	UPROPERTY()
	UHUDShipLabelWidget* ShipLabelWidget;
	APlayerController* ShipLabelPC = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars, EditAnywhere)
	FVector2D ShipLabelPositionOffset{0.0f, 40.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AccelByteWars)
	TSubclassOf<UHUDShipLabelWidget> ShipLabelWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars, EditAnywhere)
	UInGameItemDataAsset* DefaultSkinAsset;
	UInGameItemDataAsset* SkinAsset;

	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars, EditAnywhere)
	UInGameItemDataAsset* DefaultColorAsset;
	UInGameItemDataAsset* ColorAsset;

	UPROPERTY(BlueprintReadOnly, Category = AccelByteWars, EditAnywhere)
	UInGameItemDataAsset* DefaultMissileTrailFxAsset;
	UInGameItemDataAsset* MissileTrailFxAsset;
};
