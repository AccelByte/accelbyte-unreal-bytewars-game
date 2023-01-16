// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "AccelByteWarsFxActor.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsFxActor : public AActor
{
	GENERATED_BODY()

	AAccelByteWarsFxActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void DestroySelfOnParticleSystemFinished(UNiagaraComponent* Component);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UNiagaraComponent* ParticleSystem;

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
};
