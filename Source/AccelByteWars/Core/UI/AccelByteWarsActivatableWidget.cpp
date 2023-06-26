// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/AccelByteWarsActivatableWidget.h"
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
		GeneratedWidgets = DefaultObj->GeneratedWidgets;
	}
#endif
}

void UAccelByteWarsActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!bIsAlreadyInitialized) 
	{
		bIsAlreadyInitialized = true;

		// Set visible only if the associate Tutorial Module is active.
		if (AssociateTutorialModule)
		{
			const bool bIsTutorialModuleActive = AssociateTutorialModule->IsActiveAndDependenciesChecked();
			SetVisibility(bIsTutorialModuleActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}

		// Initialize the generated widgets.
		SetGeneratedWidgetContainers();
		InitializeGeneratedWidgets();
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
	GeneratedWidgets.RemoveAll([](const FTutorialModuleGeneratedWidget& Temp)
	{
		return Temp.OwnerTutorialModule == nullptr;
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

void UAccelByteWarsActivatableWidget::InitializeGeneratedWidgets()
{
	if (!IsVisible()) 
	{
		return;
	}

	// Get the default button class that will be used to spawn either entry button or action button.
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	// Sort the generated widget based on spawn order.
	GeneratedWidgets.Sort([](const FTutorialModuleGeneratedWidget& Widget1, const FTutorialModuleGeneratedWidget& Widget2)
	{
		return Widget1.SpawnOrder < Widget2.SpawnOrder;
	});

	// Initialize the generated widgets.
	for (FTutorialModuleGeneratedWidget& GeneratedWidget : GeneratedWidgets)
	{
		if ((!GeneratedWidget.OwnerTutorialModule || !GeneratedWidget.OwnerTutorialModule->IsActiveAndDependenciesChecked()) ||
			(GeneratedWidget.OtherTutorialModule && !GeneratedWidget.OtherTutorialModule->IsActiveAndDependenciesChecked()))
		{
			UE_LOG(LogTemp, Log, TEXT("Tutorial Module Data Asset is not active. Cannot initialize the generated widget."));
			continue;
		}

		if (!GeneratedWidget.TargetWidgetClass || GeneratedWidget.TargetWidgetClass != GetClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("Target Widget Class is null. Cannot initialize the generated widget."));
			continue;
		}

		// Get valid widget container.
		if (!ensure(GeneratedWidgetContainers.IsValidIndex(GeneratedWidget.TargetWidgetContainerIndex)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container index is out of bound. Cannot initialize the widget."));
			continue;
		}
		UPanelWidget* WidgetContainer = GeneratedWidgetContainers[GeneratedWidget.TargetWidgetContainerIndex];
		if (!ensure(WidgetContainer))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container is null. Cannot initialize the widget."));
			continue;
		}

		// Initialize the widget based on its type.
		if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON)
		{
			// Set valid entry widget class.
			TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = GeneratedWidget.OwnerTutorialModule->GetTutorialModuleUIClass();
			if (GeneratedWidget.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
			{
				EntryWidgetClass = (!GeneratedWidget.OwnerTutorialModule->IsStarterModeActive()) ? GeneratedWidget.DefaultTutorialModuleWidgetClass : GeneratedWidget.StarterTutorialModuleWidgetClass;
			}
			if (!EntryWidgetClass)
			{
				UE_LOG(LogTemp, Log, TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
				continue;
			}

			// Spawn the entry button.
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = GenerateButton(GeneratedWidget.ButtonText, *WidgetContainer);
			Button.Get()->OnClicked().AddWeakLambda(this, [BaseUIWidget, EntryWidgetClass]()
			{
				BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, EntryWidgetClass);
			});
			GeneratedWidget.GenerateWidgetRef = Button.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET)
		{
			// Set valid widget class to be generated.
			TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass = GeneratedWidget.OwnerTutorialModule->GetTutorialModuleUIClass();
			if (GeneratedWidget.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
			{
				WidgetClass = (!GeneratedWidget.OwnerTutorialModule->IsStarterModeActive()) ? GeneratedWidget.DefaultTutorialModuleWidgetClass : GeneratedWidget.StarterTutorialModuleWidgetClass;
			}
			if (!WidgetClass)
			{
				UE_LOG(LogTemp, Log, TEXT("Widget class is null. Cannot initialize the generated the Tutorial Module's widget."));
				continue;
			}

			const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = GenerateWidget(WidgetClass, *WidgetContainer);
			GeneratedWidget.GenerateWidgetRef = Widget.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON)
		{
			// Set valid entry widget class.
			TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = GeneratedWidget.OtherTutorialModule->GetTutorialModuleUIClass();
			if (GeneratedWidget.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
			{
				EntryWidgetClass = (!GeneratedWidget.OtherTutorialModule->IsStarterModeActive()) ? GeneratedWidget.DefaultTutorialModuleWidgetClass : GeneratedWidget.StarterTutorialModuleWidgetClass;
			}
			if (!EntryWidgetClass)
			{
				UE_LOG(LogTemp, Log, TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
				continue;
			}

			// Spawn the entry button.
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = GenerateButton(GeneratedWidget.ButtonText, *WidgetContainer);
			Button.Get()->OnClicked().AddWeakLambda(this, [BaseUIWidget, EntryWidgetClass]()
			{
				BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, EntryWidgetClass);
			});
			GeneratedWidget.GenerateWidgetRef = Button.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET)
		{
			// Set valid widget class to be generated.
			TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass = GeneratedWidget.OtherTutorialModule->GetTutorialModuleUIClass();
			if (GeneratedWidget.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
			{
				WidgetClass = (!GeneratedWidget.OtherTutorialModule->IsStarterModeActive()) ? GeneratedWidget.DefaultTutorialModuleWidgetClass : GeneratedWidget.StarterTutorialModuleWidgetClass;
			}
			if (!WidgetClass)
			{
				UE_LOG(LogTemp, Log, TEXT("Widget class is null. Cannot initialize the generated the Tutorial Module's widget."));
				continue;
			}

			const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = GenerateWidget(WidgetClass, *WidgetContainer);
			GeneratedWidget.GenerateWidgetRef = Widget.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON)
		{
			// Spawn the entry button.
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = GenerateButton(GeneratedWidget.ButtonText, *WidgetContainer);
			Button.Get()->OnClicked().AddWeakLambda(this, [BaseUIWidget, GeneratedWidget]()
			{
				BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, GeneratedWidget.GenericWidgetClass);
			});
			GeneratedWidget.GenerateWidgetRef = Button.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET)
		{
			// Spawn widget.
			const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = GenerateWidget(GeneratedWidget.GenericWidgetClass, *WidgetContainer);
			GeneratedWidget.GenerateWidgetRef = Widget.Get();
		}
		else if (GeneratedWidget.WidgetType == ETutorialModuleGeneratedWidgetType::ACTION_BUTTON)
		{
			// Spawn the action button.
			const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = GenerateButton(GeneratedWidget.ButtonText, *WidgetContainer);
			Button->OnClicked().AddWeakLambda(this, [&GeneratedWidget]()
			{
				if (!GeneratedWidget.ButtonAction.IsBound())
				{
					UE_LOG(LogTemp, Warning, TEXT("Tutorial Module's Button Action event is not bound."));
				}

				GeneratedWidget.ButtonAction.ExecuteIfBound();
			});
			GeneratedWidget.GenerateWidgetRef = Button.Get();
		}
	}
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GenerateButton(const FText& ButtonText, UPanelWidget& WidgetContainer)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);
	const TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DefaultButtonClass.Get());

	// Spawn the entry button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
	Button->SetButtonText(ButtonText);
	WidgetContainer.AddChild(Button.Get());

	// Refresh button alignment since the default alignment upon a widget is spawned is "Align_Fill".
	UPanelSlot* ButtonSlot = WidgetContainer.AddChild(Button.Get());
	if (UVerticalBoxSlot* VerticalSlot = StaticCast<UVerticalBoxSlot*>(ButtonSlot))
	{
		VerticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}
	else if (UHorizontalBoxSlot* HorizontalSlot = StaticCast<UHorizontalBoxSlot*>(ButtonSlot))
	{
		HorizontalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	}

	return Button;
}

TWeakObjectPtr<UAccelByteWarsActivatableWidget> UAccelByteWarsActivatableWidget::GenerateWidget(TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, UPanelWidget& WidgetContainer)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, WidgetClass.Get()));
	WidgetContainer.AddChild(Widget.Get());

	return Widget;
}

#undef LOCTEXT_NAMESPACE