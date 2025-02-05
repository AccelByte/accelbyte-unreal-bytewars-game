// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AccelByteWarsProceduralMeshComponent.generated.h"

/**
 * Custom ProceduralMesh component
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = Rendering, meta = (BlueprintSpawnableComponent))
class ACCELBYTEWARS_API UAccelByteWarsProceduralMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

	//~UObject overridden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject overridden functions

public:

	/**
	 * @brief Call in ConstructionScript to apply the procedural mesh
	 */
	UFUNCTION(BlueprintCallable)
	void MeshSetup();

	/**
	 * @brief Update Color variable and material parameter
	 * @param InColor
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateColor(const FLinearColor InColor);

	/**
	 * @brief Set Material Glow's brightness value. Does not modify the Glow value.
	 * @param Brightness Brightness value to be set
	 */
	UFUNCTION(BlueprintCallable)
	void SetGlowBrightness(const float Brightness) const;

	UPROPERTY(EditAnywhere)
	TArray<FVector> OutlineVertices = {
		{0.0f, 50.0f, 0.0f},
		{35.0f, -35.0f, 0.0f},
		{20.0f, -45.0f, 0.0f},
		{0.0f, -35.0f, 0.0f}
	};

	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars)
	UMaterialInterface* SourceMaterial;

	UPROPERTY(EditAnywhere)
	uint8 OutlineStrokes = 7;

	UPROPERTY(EditAnywhere)
	float Glow = 50.0f;

protected:
	TArray<int32> Triangles;
	TArray<FVector> Vertices;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn), Replicated)
	FLinearColor Color = {1.0f, 1.0f, 1.0f, 0.0f};

	UPROPERTY(EditAnywhere)
	TArray<uint8> TriStripPattern = {
		0, 2, 1, 1, 2, 3
	};
};
