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

	// Set visible only if the associate Tutorial Module is active.
	if (AssociateTutorialModule)
	{
		const bool bIsTutorialModuleActive = AssociateTutorialModule->IsActiveAndDependenciesChecked();
		SetVisibility(bIsTutorialModuleActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	InitializeGeneratedWidgets();
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
void UAccelByteWarsActivatableWidget::PostLoad()
{
	Super::PostLoad();

	ValidateGeneratedWidgets();
}

void UAccelByteWarsActivatableWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ValidateGeneratedWidgets();
}

void UAccelByteWarsActivatableWidget::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	ValidateGeneratedWidgets();
}

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

void UAccelByteWarsActivatableWidget::ValidateGeneratedWidgets()
{
	if (AssociateTutorialModule && AssociateTutorialModule->GetTutorialModuleUIClass() != GetClass())
	{
		AssociateTutorialModule = nullptr;
	}
	GeneratedWidgets.RemoveAll([](const FTutorialModuleGeneratedWidget* Temp)
	{
		return Temp->OwnerTutorialModule == nullptr;
	});
}

void UAccelByteWarsActivatableWidget::InitializeGeneratedWidgets()
{
	if (!IsVisible()) 
	{
		return;
	}

	// Remove old generated widgets.
	for (UUserWidget* OldGeneratedWidget : GeneratedWidgetPool)
	{
		OldGeneratedWidget->RemoveFromParent();
	}
	GeneratedWidgetPool.Empty();

	// Get the default button class that will be used to spawn either entry button or action button.
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	// Initialize the generated widgets.
	GeneratedWidgets.Sort();
	for (FTutorialModuleGeneratedWidget* GeneratedWidget : GeneratedWidgets)
	{
		if ((!GeneratedWidget->OwnerTutorialModule || !GeneratedWidget->OwnerTutorialModule->IsActiveAndDependenciesChecked()) ||
			(GeneratedWidget->OtherTutorialModule && !GeneratedWidget->OtherTutorialModule->IsActiveAndDependenciesChecked()))
		{
			UE_LOG(LogTemp, Log, TEXT("Tutorial Module Data Asset is not active. Cannot initialize the generated widget."));
			continue;
		}

		// Get valid widget container.
		if (!ensure(GetGeneratedWidgetContainers().IsValidIndex(GeneratedWidget->TargetWidgetContainerIndex)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container index is out of bound. Cannot initialize the widget."));
			continue;
		}
		UPanelWidget* WidgetContainer = GetGeneratedWidgetContainers()[GeneratedWidget->TargetWidgetContainerIndex];
		if (!ensure(WidgetContainer))
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module widget's Target Widget Container is null. Cannot initialize the widget."));
			continue;
		}

		// Initialize the widget based on its type.
		if (GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON ||
			GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON ||
			GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON)
		{
			GenerateEntryButton(*GeneratedWidget, *WidgetContainer);
		}
		else if (GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET ||
			GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET ||
			GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET)
		{
			GenerateWidget(*GeneratedWidget, *WidgetContainer);
		}
		else if (GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::ACTION_BUTTON)
		{
			GenerateActionButton(*GeneratedWidget, *WidgetContainer);
		}
	}
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GenerateEntryButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);
	const TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DefaultButtonClass.Get());

	// Set valid entry widget class.
	UTutorialModuleDataAsset* SourceTutorialModule = nullptr;
	TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = nullptr;
	switch (Metadata.WidgetType)
	{
	case ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON:
		SourceTutorialModule = Metadata.OwnerTutorialModule;
		break;
	case ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON:
		SourceTutorialModule = Metadata.OtherTutorialModule;
		break;
	default:
		EntryWidgetClass = Metadata.GenericWidgetClass;
		break;
	}
	
	if (SourceTutorialModule) 
	{
		EntryWidgetClass = SourceTutorialModule->GetTutorialModuleUIClass();
		if (Metadata.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
		{
			EntryWidgetClass = (!SourceTutorialModule->IsStarterModeActive()) ? Metadata.DefaultTutorialModuleWidgetClass : Metadata.StarterTutorialModuleWidgetClass;
		}
	}

	if (!EntryWidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
		return nullptr;
	}

	// Spawn the entry button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
	Button->SetButtonText(Metadata.ButtonText);
	Button->OnClicked().AddWeakLambda(this, [BaseUIWidget, EntryWidgetClass]()
	{
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, EntryWidgetClass);
	});
	WidgetContainer.AddChild(Button.Get());
	Metadata.GenerateWidgetRef = Button.Get();

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

	GeneratedWidgetPool.Add(Button.Get());

	return Button;
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GenerateActionButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);
	const TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DefaultButtonClass.Get());

	// Spawn the action button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
	Button->SetButtonText(Metadata.ButtonText);
	Button->OnClicked().AddWeakLambda(this, [&Metadata]()
	{
		if (!Metadata.ButtonAction.IsBound())
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutorial Module's Button Action with id {%s} is clicked but doesn't have any action."), *Metadata.WidgetId);
		}

		Metadata.ButtonAction.ExecuteIfBound();
	});
	WidgetContainer.AddChild(Button.Get());
	Metadata.GenerateWidgetRef = Button.Get();

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

	GeneratedWidgetPool.Add(Button.Get());

	return Button;
}

TWeakObjectPtr<UAccelByteWarsActivatableWidget> UAccelByteWarsActivatableWidget::GenerateWidget(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer)
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	// Set valid entry widget class.
	UTutorialModuleDataAsset* SourceTutorialModule = nullptr;
	TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass = nullptr;
	switch (Metadata.WidgetType)
	{
	case ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET:
		SourceTutorialModule = Metadata.OwnerTutorialModule;
		break;
	case ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET:
		SourceTutorialModule = Metadata.OtherTutorialModule;
		break;
	default:
		WidgetClass = Metadata.GenericWidgetClass;
		break;
	}

	if (SourceTutorialModule)
	{
		WidgetClass = SourceTutorialModule->GetTutorialModuleUIClass();
		if (Metadata.TutorialModuleWidgetClassType == ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
		{
			WidgetClass = (!SourceTutorialModule->IsStarterModeActive()) ? Metadata.DefaultTutorialModuleWidgetClass : Metadata.StarterTutorialModuleWidgetClass;
		}
	}

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("Widget class is null. Cannot initialize the generated the Tutorial Module's widget."));
		return nullptr;
	}

	// Spawn the widget.
	const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, WidgetClass.Get()));
	WidgetContainer.AddChild(Widget.Get());
	Metadata.GenerateWidgetRef = Widget.Get();

	GeneratedWidgetPool.Add(Widget.Get());

	return Widget;
}

#undef LOCTEXT_NAMESPACE