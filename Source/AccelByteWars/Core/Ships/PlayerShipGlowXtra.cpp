// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Ships/PlayerShipGlowXtra.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerShipGlowXtra::APlayerShipGlowXtra()
{
	AccelByteWarsProceduralMesh->OutlineVertices = {
		{0.0f, 50.0f, 0.0f},
		{45.0f, -35.0f, 0.0f},
		{20.0f, -45.0f, 0.0f},
		{0.0f, -35.0f, 0.0f}
	};

	// GlowXtra gets more 'Glow' value
	AccelByteWarsProceduralMesh->Glow = 100.0f;
	AccelByteWarsProceduralMesh->OutlineStrokes = 10;
	AccelByteWarsProceduralMesh->SetIsReplicated(true);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerShipGlowXtra::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerShipGlowXtra::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerShipGlowXtra::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerShipGlowXtra, Color);
}

void APlayerShipGlowXtra::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = InColor;

	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);

	OnRepNotify_Color();
}

void APlayerShipGlowXtra::OnRepNotify_Color()
{
	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(Color);
}

