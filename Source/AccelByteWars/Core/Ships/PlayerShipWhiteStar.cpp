// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Ships/PlayerShipWhiteStar.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerShipWhiteStar::APlayerShipWhiteStar()
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
void APlayerShipWhiteStar::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerShipWhiteStar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerShipWhiteStar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerShipWhiteStar, Color);
}

void APlayerShipWhiteStar::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = FLinearColor::White;

	// White Star overrides any given color value with 'white'
	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(FLinearColor::White);

	OnRepNotify_Color();
}

void APlayerShipWhiteStar::OnRepNotify_Color()
{
	// White Star overrides any given color value with 'white'
	if (AccelByteWarsProceduralMesh != nullptr)
		AccelByteWarsProceduralMesh->UpdateColor(FLinearColor::White);
}