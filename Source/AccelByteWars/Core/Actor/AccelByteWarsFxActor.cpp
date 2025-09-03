// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Actor/AccelByteWarsFxActor.h"

#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

// Sets default values
AAccelByteWarsFxActor::AAccelByteWarsFxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	bReplicates = true;

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

	// Bind OnSystemFinished callback on all clients for proper camera tracking
	if (bDestroyOnParticleSystemFinished && !IsRunningDedicatedServer())
	{
		ParticleSystem->OnSystemFinished.AddUniqueDynamic(this, &ThisClass::OnParticleSystemFinishedLocal);
	}

	// Set up failsafe timer for camera tracking (5 seconds)
	if (bShouldTrackForCameraZoom)
	{
		GetWorldTimerManager().SetTimer(
			CameraTrackingTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]() {
				bShouldTrackForCameraZoom = false;
				if (HasAuthority())
				{
					ForceNetUpdate();
				}
			}),
			5.0f, false);
	}
}

void AAccelByteWarsFxActor::SetNiagaraFxColor(const FLinearColor& InColor)
{
	NiagaraFxColor = InColor;
	ParticleSystem->SetVariableLinearColor(FName(NiagaraVariableColorName), NiagaraFxColor);
}

void AAccelByteWarsFxActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsFxActor, NiagaraFxColor);
	DOREPLIFETIME(AAccelByteWarsFxActor, bShouldTrackForCameraZoom);
}

void AAccelByteWarsFxActor::OnRep_NiagaraFxColor()
{
	ParticleSystem->SetVariableLinearColor(FName(NiagaraVariableColorName), NiagaraFxColor);
}

void AAccelByteWarsFxActor::OnRep_ShouldTrackForCameraZoom()
{
	// Clear the failsafe timer when camera tracking is disabled via replication
	if (!bShouldTrackForCameraZoom && CameraTrackingTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CameraTrackingTimer);
	}
}

void AAccelByteWarsFxActor::OnParticleSystemFinishedLocal(UNiagaraComponent* Component)
{
	// Disable camera tracking immediately for all clients
	bShouldTrackForCameraZoom = false;

	// Clear the failsafe timer
	if (CameraTrackingTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CameraTrackingTimer);
	}

	if (HasAuthority())
	{
		// Force network update and destroy on server
		ForceNetUpdate();
		Destroy();
	}
	else
	{
		// Only call RPC if we have a valid owner connection
		if (GetOwner() && GetOwner()->GetNetConnection())
		{
			DestroySelfOnParticleSystemFinished(Component);
		}
	}
}

void AAccelByteWarsFxActor::DestroySelfOnParticleSystemFinished_Implementation(UNiagaraComponent* Component)
{
	Destroy();
}
