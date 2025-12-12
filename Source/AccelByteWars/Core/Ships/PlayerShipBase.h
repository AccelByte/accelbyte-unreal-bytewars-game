// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "PlayerShipBase.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API APlayerShipBase : public AActor, public IInGameItemInterface
{
	GENERATED_BODY()

public:
	APlayerShipBase();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = AccelByteWars)
	UStaticMeshComponent* GetShipMesh() { return ShipMesh; }

	UFUNCTION(BlueprintPure, Category = AccelByteWars)
	const FLinearColor& GetColor() { return Color; }

	UFUNCTION(BlueprintPure, Category = AccelByteWars)
	const UTexture2D* GetAlphaTexture() { return AlphaTexture; }

	UFUNCTION(BlueprintPure, Category = AccelByteWars)
	const UTexture2D* GetColorTexture() { return ColorTexture; }

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void SetColor(const FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void SetAlphaTexture(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void SetColorTexture(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = AccelByteWars)
	void SetGlowModifier(const float Modifier);

	virtual void OnEquip() override;
	virtual void OnUse() override;

protected:
	UFUNCTION()
	void OnRepNotify_Color();

	UFUNCTION()
	void OnRepNotify_GlowModifier();

	UFUNCTION()
	void OnRepNotify_AlphaTexture();

	UFUNCTION()
	void OnRepNotify_ColorTexture();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UMaterialInterface* SourceMaterial;

	UPROPERTY(EditDefaultsOnly, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_AlphaTexture)
	UTexture2D* AlphaTexture;

	UPROPERTY(EditDefaultsOnly, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_ColorTexture)
	UTexture2D* ColorTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	float Size = 1.25f;

	UPROPERTY(BlueprintReadWrite, Category = AccelByteWars)
	UStaticMeshComponent* ShipMesh = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* ShipMaterial;

	UPROPERTY(ReplicatedUsing = OnRepNotify_GlowModifier)
	float GlowModifier = 1.0f;
};
