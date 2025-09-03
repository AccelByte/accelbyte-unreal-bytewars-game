// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AccelByteWarsMissileTrail.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogMissileTrail, Log, All);

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsMissileTrail : public AActor
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;

public:
	// Sets default values for this actor's properties
	AAccelByteWarsMissileTrail();

protected:
	//~UObject overridden functions
	virtual void BeginPlay() override;
	//~End of UObject overridden functions

	UFUNCTION()
	void OnOwnerDestroyed(AActor* InActor);

public:
	//~UObject overridden functions
	virtual void Tick(float DeltaTime) override;
	//~End of UObject overridden functions

	/**
	 * @brief Current reference to the UNiagaraComponent missile trail
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UNiagaraComponent* MissileTrail = nullptr;

	/**
	 * @brief Current Alpha color used
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float CurrentAlpha = 0.02f;

	/**
	 * @brief Current Alpha color desired
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float WantedAlpha = 0.02f;

	/**
	 * @brief Rate to interpolate between the current Alpha and the WantedAlpha
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float AlphaRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float DelayedFadeOutTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	float DelayedFadeOutCurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = AccelByteWars)
	bool bFadeOutDelayStarted = false;

	/**
	 * @brief Starts the missile trail fade out sequence
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void TriggerFadeOut();

	/**
	 * @brief Lets other players know the color of the missile trail
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
	void Server_SetColor(FLinearColor InColor);

protected:
	/**
	 * @brief Current missile trail color
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
	FLinearColor TrailColor = FLinearColor::Yellow;

	/**
	 * @brief Generic on rep notify for the missile trail color
	 */
	UFUNCTION()
	void OnRepNotify_Color();

	/**
	 * @brief Returns true if the missile trail is faded out
	 */
	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	bool IsFadeOut();

	/**
	 * @brief Set the alpha value of the trail.
	 */
	void SetRibbonAlpha(const float Alpha) const;

private:
	// Ensures we only bind once to the owner's OnDestroyed delegate when Owner becomes available on clients.
	bool bOwnerDestroyBound = false;
};
