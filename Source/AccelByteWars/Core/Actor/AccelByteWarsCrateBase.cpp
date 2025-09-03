// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsCrateBase.h"

#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/Components/AccelByteWarsProceduralMeshComponent.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"

AAccelByteWarsCrateBase::AAccelByteWarsCrateBase()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	// In the blueprint, set the static mesh to plane
	MeshComponent->SetWorldScale3D(FVector::One() * 0.5f);
	RootComponent = MeshComponent;

	AccelByteWarsGameplayObjectComponent = CreateDefaultSubobject<UAccelByteWarsGameplayObjectComponent>(TEXT("AccelByteWarsGameplayObjectComponent"));
	AccelByteWarsGameplayObjectComponent->ObjectType = EGameplayObjectType::PICKUP;
	AccelByteWarsGameplayObjectComponent->Radius = 1.f;
}

void AAccelByteWarsCrateBase::BeginPlay()
{
	Super::BeginPlay();

	if (AAccelByteWarsInGameGameMode* GameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->SetupGameplayObject(this);
	}
}

void AAccelByteWarsCrateBase::UpdateMesh()
{
	if (SourceMaterial)
	{
		if (Material == nullptr)
		{
			Material = MeshComponent->CreateDynamicMaterialInstance(0, SourceMaterial);
		}

		if (Material != nullptr && Material->IsValidLowLevel())
		{
			Material->SetVectorParameterValue(FName("EmissiveColour"), Color);
			Material->SetScalarParameterValue(FName("Glow"), Glow);
			Material->SetTextureParameterValue(FName("AlphaTexture"), AlphaTexture);
		}
	}
}

void AAccelByteWarsCrateBase::OnPicked_Implementation(AController* AffectedController)
{
	if (bIsAlreadyPicked)
	{
		return;
	}

	// Apply GameplayEffect if valid
	if (GameplayEffectClass && AffectedController && AffectedController->GetPawn())
	{
		if (AAccelByteWarsPlayerState* PlayerState = AffectedController->GetPlayerState<AAccelByteWarsPlayerState>())
		{
			if (UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
				}
			}
		}
	}

	UGameplayStatics::PlaySound2D(GetWorld(), PickedUpSound);
	bIsAlreadyPicked = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() {
		Destroy();
	}));
}
