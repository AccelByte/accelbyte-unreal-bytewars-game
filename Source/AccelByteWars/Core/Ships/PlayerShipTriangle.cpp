// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Ships/PlayerShipTriangle.h"

// Sets default values
APlayerShipTriangle::APlayerShipTriangle()
{
	AccelByteWarsProceduralMesh->OutlineVertices = {
		{0.0f, 50.0f, 0.0f},
		{45.0f, -35.0f, 0.0f},
		{20.0f, -45.0f, 0.0f},
		{0.0f, -35.0f, 0.0f}
	};

	AccelByteWarsProceduralMesh->Glow = 50.0f;
	AccelByteWarsProceduralMesh->OutlineStrokes = 7;
	AccelByteWarsProceduralMesh->SetIsReplicated(true);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerShipTriangle::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerShipTriangle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerShipTriangle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerShipTriangle, Color);
}

void APlayerShipTriangle::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = InColor;

	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);

	OnRepNotify_Color();
}

void APlayerShipTriangle::OnRepNotify_Color()
{
	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);
}
