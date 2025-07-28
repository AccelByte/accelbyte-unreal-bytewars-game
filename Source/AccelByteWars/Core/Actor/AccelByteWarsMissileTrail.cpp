// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Actor/AccelByteWarsMissileTrail.h"

#include "Core/Utilities/AccelByteWarsUtilityLog.h"

DEFINE_LOG_CATEGORY(LogMissileTrail);

// Sets default values
AAccelByteWarsMissileTrail::AAccelByteWarsMissileTrail()
{
	// Attach component in the BeginPlay to make sure the starting position is in the correct position.
	MissileTrail = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
	MissileTrail->SetAutoActivate(false);
	RootComponent = MissileTrail;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAccelByteWarsMissileTrail::BeginPlay()
{
	Super::BeginPlay();

	// Make sure the niagara component is in the correct position before activating particle.
	if (Owner)
	{
		MissileTrail->SetWorldLocation(Owner->GetActorLocation());
	}
	else
	{
		UE_LOG_FUNC(LogMissileTrail, Warning, TEXT("Owner is null. Trail might start at incorrect location."))
	}
	MissileTrail->Activate();

	// Set component's color in case the class member was set before this. 
	OnRepNotify_Color();
	SetRibbonAlpha(CurrentAlpha);

	if (!HasAuthority())
	{
		return;
	}

	// Destroy self if owner destroyed or if there's no owner to avoid "zombie" trail.
	if (Owner)
	{
		Owner->OnDestroyed.AddDynamic(this, &ThisClass::OnOwnerDestroyed);
	}
	else
	{
		Destroy();
	}
}

void AAccelByteWarsMissileTrail::OnOwnerDestroyed(AActor* InActor)
{
	bFadeOutDelayStarted = true;
}

// Called every frame
void AAccelByteWarsMissileTrail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Delayed fade out logic.
	if (!IsFadeOut() && bFadeOutDelayStarted)
	{
		if (DelayedFadeOutCurrentTime >= DelayedFadeOutTime)
		{
			TriggerFadeOut();
		}
		else
		{
			DelayedFadeOutCurrentTime += DeltaTime;
		}
	}

	// Fade out logic.
	if (!FMath::IsNearlyEqual(CurrentAlpha, WantedAlpha, 0.00001))
	{
		CurrentAlpha = FMath::Lerp(CurrentAlpha, WantedAlpha, DeltaTime * AlphaRate);
		SetRibbonAlpha(CurrentAlpha);
	}

	// Destroy logic.
	// Lerp function used, uses DeltaTime as the weight. CurrentAlpha might never reach exactly 0.
	if (FMath::IsNearlyEqual(CurrentAlpha, 0.0, 0.00001))
	{
		Destroy();
	}
}

void AAccelByteWarsMissileTrail::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsMissileTrail, TrailColor);
	DOREPLIFETIME(AAccelByteWarsMissileTrail, bFadeOutDelayStarted);
}

void AAccelByteWarsMissileTrail::Destroyed()
{
	RootComponent->Deactivate();

	Super::Destroyed();
}

bool AAccelByteWarsMissileTrail::IsFadeOut()
{
	if (CurrentAlpha <= 0.0f)
		return true;

	return false;
}

void AAccelByteWarsMissileTrail::TriggerFadeOut()
{
	WantedAlpha = 0.0f;
}

void AAccelByteWarsMissileTrail::Server_SetColor_Implementation(FLinearColor InColor)
{
	TrailColor = InColor;
	OnRepNotify_Color();
}

void AAccelByteWarsMissileTrail::OnRepNotify_Color()
{
	if (!MissileTrail)
	{
		return;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	MissileTrail->SetVariableLinearColor("RibbonColour", TrailColor);
#else
	MissileTrail->SetNiagaraVariableLinearColor("RibbonColour", TrailColor);
#endif
}

void AAccelByteWarsMissileTrail::SetRibbonAlpha(const float Alpha) const
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	MissileTrail->SetVariableFloat("RibbonAlphaScale", Alpha);
#else
	MissileTrail->SetNiagaraVariableFloat("RibbonAlphaScale", Alpha);
#endif
}

