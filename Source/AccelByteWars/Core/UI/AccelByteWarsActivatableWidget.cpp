// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"

#include "Editor/WidgetCompilerLog.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

UAccelByteWarsActivatableWidget::UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAccelByteWarsActivatableWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

#if WITH_EDITOR
	const UAccelByteWarsActivatableWidget* DefaultObj = Cast<UAccelByteWarsActivatableWidget>(GetClass()->GetDefaultObject());
	if (DefaultObj)
	{
		AssociateTutorialModule = DefaultObj->AssociateTutorialModule;
		DissociateTutorialModuleWidgets = DefaultObj->DissociateTutorialModuleWidgets;
	}
#endif
}

void UAccelByteWarsActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!bIsTutorialModuleWidgetsInitialized) 
	{
		bIsTutorialModuleWidgetsInitialized = true;
		SetTutorialModuleWidgetContainers();
		LoadTutorialModuleWidgetConnection();
	}
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

void UAccelByteWarsActivatableWidget::PostLoad()
{
	Super::PostLoad();

	if (AssociateTutorialModule && AssociateTutorialModule->GetTutorialModuleUIClass() != GetClass())
	{
		AssociateTutorialModule = nullptr;
	}
	DissociateTutorialModuleWidgets.RemoveAll([](const FTutorialModuleWidgetConnection& Temp)
	{
		return Temp.SourceTutorialModule == nullptr;
	});
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
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
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
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void UAccelByteWarsActivatableWidget::SetInputModeToGameOnly()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to change input mode. Player controller is not valid or already destroyed."));
		return;
	}

	// Set input mode to game only mode.
	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = false;
}

void UAccelByteWarsActivatableWidget::LoadTutorialModuleWidgetConnection()
{
	// Connect Other Tutorial Module Widgets to This Tutorial Module.
	if (AssociateTutorialModule)
	{
		const bool bIsTutorialModuleActive = AssociateTutorialModule->IsActiveAndDependenciesChecked();
		SetVisibility(bIsTutorialModuleActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bIsTutorialModuleActive) 
		{
			InitializeTutorialModuleWidgets(AssociateTutorialModule->OtherTutorialModuleWidgetsToThisModuleWidgetConnections);
		}
	}

	// Connect This Tutorial Module Widgets to Non-Tutorial Module.
	DissociateTutorialModuleWidgets.Sort([](const FTutorialModuleWidgetConnection& Conn1, const FTutorialModuleWidgetConnection& Conn2)
	{
		return Conn1.PriorityOrder < Conn2.PriorityOrder;
	});
	InitializeTutorialModuleWidgets(DissociateTutorialModuleWidgets);
}

void UAccelByteWarsActivatableWidget::InitializeTutorialModuleWidgets(TArray<FTutorialModuleWidgetConnection>& TutorialModuleWidgets)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	const TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DefaultButtonClass.Get());

	for (const FTutorialModuleWidgetConnection& Connection : TutorialModuleWidgets)
	{
		if (!Connection.SourceTutorialModule || !Connection.SourceTutorialModule->IsActiveAndDependenciesChecked())
		{
			UE_LOG(LogTemp, Log, TEXT("Tutorial Module widget's Tutorial Module Data Asset is not active. Cannot initialize the widget."));
			continue;
		}

		if (!ensure(Connection.TargetUIClass.Get()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target UI Class is null. Cannot initialize the widget."));
			continue;
		}

		// Get valid widget container.
		if (!ensure(TutorialModuleWidgetContainers.IsValidIndex(Connection.TargetWidgetContainerIndex)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container index is out of bound. Cannot initialize the widget."));
			continue;
		}
		UPanelWidget* WidgetContainer = TutorialModuleWidgetContainers[Connection.TargetWidgetContainerIndex];
		if (!ensure(WidgetContainer))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container is null. Cannot initialize the widget."));
			continue;
		}

		// Initialize the widget based on its type.
		if (Connection.WidgetType == ETutorialModuleWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON)
		{
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
			Button->OnClicked().AddWeakLambda(this, [Connection, BaseUIWidget]()
			{
				BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, Connection.SourceTutorialModule->GetTutorialModuleUIClass());
			});
			Button->SetButtonText(Connection.EntryButtonText);
			WidgetContainer->AddChild(Button.Get());

			UPanelSlot* ButtonSlot = WidgetContainer->AddChild(Button.Get());
			if (UVerticalBoxSlot* VerticalSlot = StaticCast<UVerticalBoxSlot*>(ButtonSlot))
			{
				VerticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			}
			else if (UHorizontalBoxSlot* HorizontalSlot = StaticCast<UHorizontalBoxSlot*>(ButtonSlot))
			{
				HorizontalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			}
		}
		else if (Connection.WidgetType == ETutorialModuleWidgetType::TUTORIAL_MODULE_DEFAULT_UI)
		{
			const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, Connection.SourceTutorialModule->GetTutorialModuleUIClass().Get()));
			WidgetContainer->AddChild(Widget.Get());
		}
		else if (Connection.WidgetType == ETutorialModuleWidgetType::OTHER_UI_ENTRY_BUTTON)
		{
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
			Button->OnClicked().AddWeakLambda(this, [Connection, BaseUIWidget]()
			{
				BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, Connection.OtherUIClass);
			});
			Button->SetButtonText(Connection.EntryButtonText);
			
			UPanelSlot* ButtonSlot = WidgetContainer->AddChild(Button.Get());
			if (UVerticalBoxSlot* VerticalSlot = StaticCast<UVerticalBoxSlot*>(ButtonSlot))
			{
				VerticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			}
			else if (UHorizontalBoxSlot* HorizontalSlot = StaticCast<UHorizontalBoxSlot*>(ButtonSlot))
			{
				HorizontalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			}
		}
		else if (Connection.WidgetType == ETutorialModuleWidgetType::OTHER_UI)
		{
			const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, Connection.OtherUIClass));
			WidgetContainer->AddChild(Widget.Get());
		}
	}
}

#undef LOCTEXT_NAMESPACE