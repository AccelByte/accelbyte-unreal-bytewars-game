// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsInGamePlayerController.h"
#include "AccelByteWarsPlayerPawn.h"
#include "Core/UI/InGameMenu/HUD/HUDPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "CommonInputSubsystem.h"

void AAccelByteWarsInGamePlayerController::RotateShip(const float Value)
{
	AAccelByteWarsPlayerPawn* PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(GetPawn());
	if (!PlayerPawn)
	{
		return;
	}

	PlayerPawn->Server_RotatePawn(Value);
}

void AAccelByteWarsInGamePlayerController::AdjustPower(const float Value)
{
	AAccelByteWarsPlayerPawn* PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(GetPawn());
	if (!PlayerPawn)
	{
		return;
	}

	AHUDPlayer* HUD = Cast<AHUDPlayer>(GetHUD());
	if (!HUD) 
	{
		return;
	}

	const FVector Location = PlayerPawn->GetActorLocation();
	const FLinearColor Color = PlayerPawn->GetPawnColor();

	FVector2D ScreenLocation;
	ProjectWorldLocationToScreen(Location, ScreenLocation, false);

	HUD->UpdatePowerBarUI(ScreenLocation, Color);
	PlayerPawn->OnPlayerInputThisFrame();
	PlayerPawn->Server_AdjustFirePower(Location, Value);
}

void AAccelByteWarsInGamePlayerController::FireMissile()
{
	if (AAccelByteWarsPlayerPawn * PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(GetPawn()))
	{
		PlayerPawn->Server_FireMissile();
	}
}

void AAccelByteWarsInGamePlayerController::UsePowerUp()
{
	if (AAccelByteWarsPlayerPawn* PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(GetPawn()))
	{
		PlayerPawn->ServerActivatePowerUp();
	}
}

void AAccelByteWarsInGamePlayerController::ToggleMouseCapture()
{
	if (!GetLocalPlayer()) 
	{
		return;
	}

	const UCommonInputSubsystem* CommonInputSubsystem = GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>();
	if (!CommonInputSubsystem) 
	{
		return;
	}

	const EMouseCaptureMode MouseCaptureMode = UGameplayStatics::GetViewportMouseCaptureMode(this);

	// Release mouse capture
	if (CommonInputSubsystem->IsInputMethodActive(ECommonInputType::MouseAndKeyboard)) 
	{
		if (MouseCaptureMode != EMouseCaptureMode::NoCapture) 
		{
			UGameplayStatics::SetViewportMouseCaptureMode(this, EMouseCaptureMode::NoCapture);
		}
		if (!bShowMouseCursor) 
		{
			SetShowMouseCursor(true);
		}
	}
	// Capture mouse.
	else 
	{
		if (MouseCaptureMode != EMouseCaptureMode::CapturePermanently)
		{
			UGameplayStatics::SetViewportMouseCaptureMode(this, EMouseCaptureMode::CapturePermanently);
		}
		if (bShowMouseCursor)
		{
			SetShowMouseCursor(false);
		}
	}
}
