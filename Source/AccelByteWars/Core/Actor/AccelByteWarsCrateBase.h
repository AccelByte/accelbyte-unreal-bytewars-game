// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AccelByteWarsCrateBase.generated.h"

class UAccelByteWarsGameplayObjectComponent;
class UAccelByteWarsProceduralMeshComponent;
class UGameplayEffect;

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsCrateBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAccelByteWarsCrateBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void UpdateMesh();

	UFUNCTION(BlueprintNativeEvent, Category = AccelByteWars)
	void OnPicked(AController* AffectedController);

	UTexture2D* GetAlphaTexture() { return AlphaTexture; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UStaticMeshComponent* MeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UAccelByteWarsGameplayObjectComponent* AccelByteWarsGameplayObjectComponent = nullptr;

protected:
	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UMaterialInterface* SourceMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	FLinearColor Color;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	float Glow{ 10 };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UTexture2D* AlphaTexture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	float CrateLifetimeInSeconds{ 30 };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	USoundBase* PickedUpSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

private:
	bool bIsAlreadyPicked{ false };
};
