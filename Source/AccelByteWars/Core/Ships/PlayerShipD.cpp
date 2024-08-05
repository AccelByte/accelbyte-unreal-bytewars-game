// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Ships/PlayerShipD.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerShipD::APlayerShipD()
{
	// "D" Shape
	AccelByteWarsProceduralMesh->OutlineVertices = {
		{0.0f, 36.0f, 0.0f},
		{4.0f, 31.0f, 0.0f},
		{23.0f, 23.0f, 0.0f},
		{30.0f, 19.0f, 0.0f},
		{40.0f, 11.0f, 0.0f},
		{45.0f, -2.0f, 0.0f},
		{45.0f, -20.0f, 0.0f},
		{45.0f, -20.0f, 0.0f},
		{0.0f, -22.0f, 0.0f}
	};

	AccelByteWarsProceduralMesh->Glow = 50.0f;
	AccelByteWarsProceduralMesh->OutlineStrokes = 7;
	AccelByteWarsProceduralMesh->SetIsReplicated(true);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerShipD::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerShipD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerShipD::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerShipD, Color);
}

void APlayerShipD::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = InColor;

	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);

	OnRepNotify_Color();
}

void APlayerShipD::OnRepNotify_Color()
{
	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);
}

