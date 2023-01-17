// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/Actor/AccelByteWarsFxActor.h"

// Sets default values
AAccelByteWarsFxActor::AAccelByteWarsFxActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	NewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = NewRoot;

	ParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
	ParticleSystem->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AAccelByteWarsFxActor::BeginPlay()
{
	Super::BeginPlay();

	if (bDestroyOnParticleSystemFinished)
	{
		ParticleSystem->OnSystemFinished.AddUniqueDynamic(this, &ThisClass::DestroySelfOnParticleSystemFinished);
	}
}

void AAccelByteWarsFxActor::DestroySelfOnParticleSystemFinished(UNiagaraComponent* Component)
{
	Destroy();
}
