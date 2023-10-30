// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Ships/PlayerShipBase.h"

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

// Sets default values
APlayerShipBase::APlayerShipBase()
{
	AccelByteWarsProceduralMesh = CreateDefaultSubobject<UAccelByteWarsProceduralMeshComponent>(TEXT("AccelByteWarsProceduralMesh"));
	RootComponent = AccelByteWarsProceduralMesh;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerShipBase::BeginPlay()
{
	Super::BeginPlay();

	AccelByteWarsProceduralMesh->MeshSetup();
}

// Called every frame
void APlayerShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}