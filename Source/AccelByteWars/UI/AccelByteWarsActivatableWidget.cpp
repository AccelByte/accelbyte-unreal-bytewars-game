// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "UI/AccelByteWarsActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"
#include "Input/CommonInputMode.h"
#include "Input/UIActionBindingHandle.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

UAccelByteWarsActivatableWidget::UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UAccelByteWarsActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EAccelByteWarsWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EAccelByteWarsWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EAccelByteWarsWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EAccelByteWarsWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

void UAccelByteWarsActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UAccelByteWarsActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UAccelByteWarsActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

void UAccelByteWarsActivatableWidget::MoveCameraToTargetLocation(const float DeltaTime, const FVector TargetLocation, const float InterpSpeed)
{
	AActor* Camera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
	if (Camera == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot move the camera to active menu. Camera is not found."));
		return;
	}

	const FVector CurrentLocation = Camera->GetActorLocation();
	const FVector DesiredLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpSpeed);
	Camera->SetActorLocation(DesiredLocation);
}

void UAccelByteWarsActivatableWidget::SetInputModeToUIOnly()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to change input mode. Player controller is not valid or already destroyed."));
		return;
	}

	// Set input mode to UI only mode.
	UWidget* InitialFocus = GetDesiredFocusTarget();
	FInputModeUIOnly InputMode;
	if (InitialFocus)
	{
		InputMode.SetWidgetToFocus(InitialFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Controller->SetInputMode(InputMode);
	Controller->bShowMouseCursor = true;
}

void UAccelByteWarsActivatableWidget::SetInputModeToGameOnly()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to change input mode. Player controller is not valid or already destroyed."));
		return;
	}

	// Set input mode to game only mode.
	FInputModeGameOnly InputMode;
	Controller->SetInputMode(InputMode);
	Controller->bShowMouseCursor = false;
}

#undef LOCTEXT_NAMESPACE