// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Components/AccelByteWarsProceduralMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void UAccelByteWarsProceduralMeshComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAccelByteWarsProceduralMeshComponent, Color);
}

void UAccelByteWarsProceduralMeshComponent::MeshSetup()
{
	if (SourceMaterial)
	{
		TArray<FVector> MirroredOutlineVertices;

		if (Material == nullptr)
		{
			Material = CreateDynamicMaterialInstance(0, SourceMaterial);
		}

		if (Material != nullptr && Material->IsValidLowLevel())
		{
			Material->SetVectorParameterValue(FName("EmissiveColour"), Color);
			Material->SetScalarParameterValue(FName("Glow"), Glow);
		}

		ClearMeshSection(0);

		Vertices.Empty();
		Triangles.Empty();

		for (const FVector& Vertex : OutlineVertices)
		{
			MirroredOutlineVertices.Add(Vertex);
		}

		const uint8 OutlineVerticesLastIndex = OutlineVertices.Num() - 1;
		for (int32 Index = (OutlineVertices.Num() - 1); Index >= 0; --Index)
		{
			if (Index == OutlineVerticesLastIndex) continue;
			MirroredOutlineVertices.Add(UKismetMathLibrary::MirrorVectorByNormal(OutlineVertices[Index], {1.0f, 0.0f, 0.0f}));
		}

		// add outline inset pairs
		for (const FVector& Vertex : MirroredOutlineVertices)
		{
			// add outline verts
			Vertices.Add(Vertex);

			// add calculated inset verts
			/*
			 * Generate Inset Vert. Currently calculated (badly) as outline->centre, normalised and scaled.
			 * TODO: adjust inset to maintain consistent stroke across shape
			 */
			FVector CalculatedVertex =
				Vertex - (UKismetMathLibrary::Normal(Vertex, 0.0001f) * UKismetMathLibrary::Conv_IntToVector(OutlineStrokes));
			Vertices.Add(CalculatedVertex);
		}

		for (int32 Index = 0; Index < MirroredOutlineVertices.Num(); ++Index)
		{
			for (const uint8 Pattern : TriStripPattern)
			{
				Triangles.Add(Pattern + (Index * 2));
			}
		}

		CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2d>(), TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	}
}

void UAccelByteWarsProceduralMeshComponent::UpdateColor(const FLinearColor InColor)
{
	Color = InColor;

	if (Material != nullptr)
		Material->SetVectorParameterValue(FName("EmissiveColour"), Color);
}

void UAccelByteWarsProceduralMeshComponent::SetGlowBrightness(const float Brightness) const
{
	if (Material)
	{
		Material->SetScalarParameterValue(FName("Glow"), Glow * Brightness);
	}
}
