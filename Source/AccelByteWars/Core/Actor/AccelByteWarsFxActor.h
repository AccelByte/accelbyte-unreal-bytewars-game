// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "AccelByteWarsFxActor.generated.h"

/**
 * @brief FX purpose actor. Will destroy it self upon Particle System finished.
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsFxActor : public AActor
{
	GENERATED_BODY()

	AAccelByteWarsFxActor();

	//~AActor overridden functions
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor overridden functions

public:
	void SetNiagaraFxColor(const FLinearColor& InColor);
	const bool ShouldTrackForCameraZoom() const { return bShouldTrackForCameraZoom; }

protected:
	UFUNCTION(Reliable, Server)
	void DestroySelfOnParticleSystemFinished(UNiagaraComponent* Component);

	UFUNCTION()
	void OnParticleSystemFinishedLocal(UNiagaraComponent* Component);

	UFUNCTION()
	void OnRep_ShouldTrackForCameraZoom();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UNiagaraComponent* ParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NiagaraVariableColorName{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_NiagaraFxColor)
	FLinearColor NiagaraFxColor{};
	UFUNCTION()
	void OnRep_NiagaraFxColor();

	/**
	 * @brief Use this instead of DefaultSceneRoot
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* NewRoot;

	/**
	 * @brief Whether to automatically destroy actor upon particle finished or not
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bDestroyOnParticleSystemFinished = true;

	/**
	 * @brief affect camera zoom out if out of bounds
	 */
	UPROPERTY(EditDefaultsOnly, Replicated, ReplicatedUsing = OnRep_ShouldTrackForCameraZoom)
	bool bShouldTrackForCameraZoom = true;

	/** Timer handle for camera tracking failsafe */
	FTimerHandle CameraTrackingTimer;
};
