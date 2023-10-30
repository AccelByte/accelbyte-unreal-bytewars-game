// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Actor/AccelByteWarsMissileTrail.h"

// Sets default values
AAccelByteWarsMissileTrail::AAccelByteWarsMissileTrail()
{
	MissileTrail = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
	MissileTrail->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RootComponent = MissileTrail;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAccelByteWarsMissileTrail::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAccelByteWarsMissileTrail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAccelByteWarsMissileTrail::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsMissileTrail, TrailColor);
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
	MissileTrail->SetNiagaraVariableLinearColor("RibbonColour", TrailColor);
}

