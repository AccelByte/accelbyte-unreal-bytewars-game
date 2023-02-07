// Fill out your copyright notice in the Description page of Project Settings.

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

protected:
	TArray<int32> Triangles;
	TArray<FVector> Vertices;

	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn), Replicated)
	FLinearColor Color = {1.0f, 1.0f, 1.0f, 0.0f};

	UPROPERTY(EditAnywhere)
	TArray<uint8> TriStripPattern = {
		0, 2, 1, 1, 2, 3
	};

	UPROPERTY(EditAnywhere)
	TArray<FVector> OutlineVertices = {
		{0.0f, 50.0f, 0.0f},
		{35.0f, -35.0f, 0.0f},
		{20.0f, -45.0f, 0.0f},
		{0.0f, -35.0f, 0.0f}
	};

	UPROPERTY(EditAnywhere)
	UMaterialInterface* SourceMaterial;

	UPROPERTY(EditAnywhere)
	uint8 OutlineStrokes = 7;

	UPROPERTY(EditAnywhere)
	float Glow = 50.0f;
};
