// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Ships/PlayerShipBase.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Net/UnrealNetwork.h"

APlayerShipBase::APlayerShipBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetWorldScale3D(FVector::One() * Size);
	RootComponent = ShipMesh;
}

void APlayerShipBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SourceMaterial)
	{
		if (!ShipMaterial)
		{
			ShipMaterial = ShipMesh->CreateDynamicMaterialInstance(0, SourceMaterial);
		}

		if (ShipMaterial && ShipMaterial->IsValidLowLevel())
		{
			ShipMesh->SetMaterial(0, ShipMaterial);

			OnRepNotify_AlphaTexture();
			OnRepNotify_ColorTexture();
			OnRepNotify_Color();
			OnRepNotify_GlowModifier();
		}
	}
}

void APlayerShipBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AlphaTexture);
	DOREPLIFETIME(ThisClass, ColorTexture);
	DOREPLIFETIME(ThisClass, Color);
	DOREPLIFETIME(ThisClass, GlowModifier);
}

void APlayerShipBase::SetColor(const FLinearColor InColor)
{
	if (!HasAuthority())
	{
		return;
	}

	Color = InColor;

	if (!IsRunningDedicatedServer())
	{
		OnRepNotify_Color();
	}
}

void APlayerShipBase::SetAlphaTexture(UTexture2D* Texture)
{
	if (!HasAuthority())
	{
		return;
	}

	AlphaTexture = Texture;

	if (!IsRunningDedicatedServer())
	{
		OnRepNotify_AlphaTexture();
	}
}

void APlayerShipBase::SetColorTexture(UTexture2D* Texture)
{
	if (!HasAuthority())
	{
		return;
	}

	ColorTexture = Texture;

	if (!IsRunningDedicatedServer())
	{
		OnRepNotify_ColorTexture();
	}
}

void APlayerShipBase::SetGlowModifier(const float Modifier)
{
	if (!HasAuthority())
	{
		return;
	}

	GlowModifier = Modifier;

	if (!IsRunningDedicatedServer())
	{
		OnRepNotify_GlowModifier();
	}
}

void APlayerShipBase::OnRepNotify_AlphaTexture()
{
	if (ShipMaterial && AlphaTexture)
	{
		ShipMaterial->SetTextureParameterValue(FName("AlphaTexture"), AlphaTexture);
	}
}

void APlayerShipBase::OnRepNotify_ColorTexture()
{
	if (ShipMaterial && ColorTexture)
	{
		ShipMaterial->SetTextureParameterValue(FName("ColorTexture"), ColorTexture);
	}
}

void APlayerShipBase::OnRepNotify_Color()
{
	if (ShipMaterial)
	{
		ShipMaterial->SetVectorParameterValue(FName("EmissiveColour"), Color);
	}
}

void APlayerShipBase::OnRepNotify_GlowModifier()
{
	if (ShipMaterial)
	{
		ShipMaterial->SetScalarParameterValue(FName("Glow"), GlowModifier);
	}
}

void APlayerShipBase::OnEquip()
{
	IInGameItemInterface::OnEquip();
}

void APlayerShipBase::OnUse()
{
	IInGameItemInterface::OnUse();
}