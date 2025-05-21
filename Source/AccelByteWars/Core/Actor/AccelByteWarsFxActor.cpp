// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Actor/AccelByteWarsFxActor.h"

// Sets default values
AAccelByteWarsFxActor::AAccelByteWarsFxActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;

	// Lower the net update frequency since this is only an FX actor
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	SetNetUpdateFrequency(5.0f);
	SetMinNetUpdateFrequency(1.0f);
#else
	NetUpdateFrequency = 5.0f;
	MinNetUpdateFrequency = 1.0f;
#endif

	NewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = NewRoot;

	ParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
	ParticleSystem->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AAccelByteWarsFxActor::BeginPlay()
{
	Super::BeginPlay();

	// actor tick not running on DS, this need to be called from the owning client
	if (bDestroyOnParticleSystemFinished && HasLocalNetOwner() && !IsRunningDedicatedServer())
	{
		ParticleSystem->OnSystemFinished.AddUniqueDynamic(this, &ThisClass::DestroySelfOnParticleSystemFinished);
	}
}

void AAccelByteWarsFxActor::DestroySelfOnParticleSystemFinished_Implementation(UNiagaraComponent* Component)
{
	Destroy();
}
