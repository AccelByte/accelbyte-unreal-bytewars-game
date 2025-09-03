// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsInGameCameraActor.h"

#include "AccelByteWarsFxActor.h"
#include "AccelByteWarsAsteroid.h"
#include "Camera/CameraComponent.h"
#include "Core/Components/AccelByteWarsGameplayObjectComponent.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AAccelByteWarsInGameCameraActor::AAccelByteWarsInGameCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAccelByteWarsInGameCameraActor::BeginPlay()
{
	Super::BeginPlay();

	// var setup
	InGameGameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
	ensure(InGameGameState);
}

void AAccelByteWarsInGameCameraActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Camera setup
	AdjustCamera();

	// Camera pulse
	PulseTarget = UKismetMathLibrary::Lerp(PulseTarget, 0.6f, (DeltaSeconds * 4.0));
	GetCameraComponent()->PostProcessSettings.BloomIntensity = PulseTarget;
}

void AAccelByteWarsInGameCameraActor::PulseBackground()
{
	PulseTarget = 3.5f;
}

void AAccelByteWarsInGameCameraActor::AdjustCamera()
{
	// var setup
	const float GameWidth = UKismetMathLibrary::Abs(InGameGameState->MaxGameBound.X - InGameGameState->MinGameBound.X);
	const float GameHeight = UKismetMathLibrary::Abs(InGameGameState->MaxGameBound.Y - InGameGameState->MinGameBound.Y);
	const float HUDHeight = GetHudHeight();

	// center camera to the center of play area and move it up to accommodate HUD
	const FVector LocationTarget = {
		InGameGameState->MinGameBound.X + (GameWidth / 2),
		InGameGameState->MinGameBound.Y + ((GameHeight + HUDHeight) / 2),
		GetCameraComponent()->GetComponentLocation().Z
	};
	GetCameraComponent()->SetWorldLocation(LocationTarget, false, nullptr, ETeleportType::TeleportPhysics);

	// check if there's actor that is outside or inside play area
	float DeltaX = 0.0f;
	float DeltaY = 0.0f;
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), Actors);
	for (const AActor* Actor : Actors)
	{
		// Skip invalid actors (destroyed, pending kill, etc.)
		if (!IsValid(Actor))
		{
			continue;
		}

		// Skip Asteroids to prevent them from affecting camera zoom
		if (Cast<AAccelByteWarsAsteroid>(Actor))
		{
			continue;
		}

		// Handle FxActor tracking check with additional safety
		const AAccelByteWarsFxActor* FxActor = Cast<AAccelByteWarsFxActor>(Actor);
		if (IsValid(FxActor) && !FxActor->ShouldTrackForCameraZoom())
		{
			continue;
		}

		if (Actor->GetComponentByClass(UAccelByteWarsGameplayObjectComponent::StaticClass()) || Cast<AAccelByteWarsFxActor>(Actor))
		{
			if (Actor->GetActorLocation().X > InGameGameState->MaxGameBound.X)
			{
				DeltaX = FMath::Max(DeltaX, FMath::Abs(Actor->GetActorLocation().X - InGameGameState->MaxGameBound.X));
			}
			if (Actor->GetActorLocation().Y > InGameGameState->MaxGameBound.Y)
			{
				DeltaY = FMath::Max(DeltaY, FMath::Abs(Actor->GetActorLocation().Y - InGameGameState->MaxGameBound.Y));
			}
			if (Actor->GetActorLocation().X < InGameGameState->MinGameBound.X)
			{
				DeltaX = FMath::Max(DeltaX, FMath::Abs(Actor->GetActorLocation().X - InGameGameState->MinGameBound.X));
			}
			if (Actor->GetActorLocation().Y < InGameGameState->MinGameBound.Y)
			{
				DeltaY = FMath::Max(DeltaY, FMath::Abs(Actor->GetActorLocation().Y - InGameGameState->MinGameBound.Y));
			}
		}
	}

	// calculate camera bound and clamp
	FVector2D MaxCamBound = {
		FMath::Max(InGameGameState->MaxGameBound.X + DeltaX, InGameGameState->MaxGameBound.X),
		FMath::Max(InGameGameState->MaxGameBound.Y + DeltaY, InGameGameState->MaxGameBound.Y)
	};
	FVector2D MinCamBound = {
		FMath::Min(InGameGameState->MinGameBound.X - DeltaX, InGameGameState->MinGameBound.X),
		FMath::Min(InGameGameState->MinGameBound.Y - DeltaY, InGameGameState->MinGameBound.Y)
	};
	MaxCamBound = {
		FMath::Min(MaxCamBound.X, InGameGameState->MaxGameBoundExtend.X),
		FMath::Min(MaxCamBound.Y, InGameGameState->MaxGameBoundExtend.Y)
	};
	MinCamBound = {
		FMath::Max(MinCamBound.X, InGameGameState->MinGameBoundExtend.X),
		FMath::Max(MinCamBound.Y, InGameGameState->MinGameBoundExtend.Y)
	};

	// fit to X or fit to Y
	const float OrthoWidthTarget = UKismetMathLibrary::Max(
		FMath::Abs(MaxCamBound.X - MinCamBound.X),
		(FMath::Abs(MaxCamBound.Y - MinCamBound.Y) + HUDHeight) * GetCameraComponent()->AspectRatio);
	GetCameraComponent()->SetOrthoWidth(FMath::Lerp(GetCameraComponent()->OrthoWidth, OrthoWidthTarget + 200.0f, 0.1f));
}
