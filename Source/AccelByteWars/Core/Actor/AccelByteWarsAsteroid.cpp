// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsAsteroid.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Components/StaticMeshComponent.h"

AAccelByteWarsAsteroid::AAccelByteWarsAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize with default properties
	Properties = FAsteroidProperties();
	SetAsteroidProperties(Properties);
}

void AAccelByteWarsAsteroid::BeginPlay()
{
	Super::BeginPlay();

	GenerateAsteroidProceduralMesh();
	InitialRotation = GetActorRotation();
}

void AAccelByteWarsAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyAsteroidRotation(DeltaTime);
}

void AAccelByteWarsAsteroid::SetAsteroidProperties(const FAsteroidProperties& InProperties)
{
	Properties = InProperties;

	MaxTimeAlive = Properties.MaxTimeAlive;
	GravitationalConstant = Properties.GravitationalConstant;

	if (AccelByteWarsGameplayObjectComponent)
	{
		AccelByteWarsGameplayObjectComponent->ObjectType = EGameplayObjectType::MISSILE; // Keep as missile for collision purposes
	}

	GenerateAsteroidProceduralMesh();
}

void AAccelByteWarsAsteroid::ApplyAsteroidRotation(float DeltaTime)
{
	// Apply continuous rotation to make the Asteroid tumble - only Yaw rotation
	CurrentRotation += Properties.RotationSpeed * DeltaTime;

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += CurrentRotation; // Only rotate around Yaw axis

	SetActorRotation(NewRotation);
}

void AAccelByteWarsAsteroid::GenerateAsteroidProceduralMesh()
{
	if (!AccelByteWarsProceduralMesh)
	{
		return;
	}

	const float BaseRadius = AccelByteWarsGameplayObjectComponent->Radius; // Scale to mesh units

	TArray<FVector> OutlineVertices{
		{ 0.0f, BaseRadius * 57.0f, 0.0f },				   // Top
		{ BaseRadius * 32.0f, BaseRadius * 58.0f, 0.0f },  // Top-right bump
		{ BaseRadius * 50.0f, BaseRadius * 34.0f, 0.0f },  // Top-right
		{ BaseRadius * 56.0f, BaseRadius * 10.0f, 0.0f },  // Right
		{ BaseRadius * 60.0f, -BaseRadius * 8.0f, 0.0f },  // Right-lower
		{ BaseRadius * 49.0f, -BaseRadius * 34.0f, 0.0f }, // Right-bottom
		{ BaseRadius * 21.0f, -BaseRadius * 56.0f, 0.0f }, // Bottom-right
		{ 0.0f, -BaseRadius * 60.0f, 0.0f }				   // Bottom center
	};

	AccelByteWarsProceduralMesh->OutlineVertices = OutlineVertices;
	AccelByteWarsProceduralMesh->UpdateColor(Color);
	AccelByteWarsProceduralMesh->MeshSetup();
}
