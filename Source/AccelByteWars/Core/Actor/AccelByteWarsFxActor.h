// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
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
	//~End of AActor overridden functions

protected:

	UFUNCTION(Reliable, Server)
	void DestroySelfOnParticleSystemFinished(UNiagaraComponent* Component);

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
