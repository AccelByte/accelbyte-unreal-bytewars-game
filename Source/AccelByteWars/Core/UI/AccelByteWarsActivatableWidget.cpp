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

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsActivatableWidget);

#define LOCTEXT_NAMESPACE "AccelByteWars"

UAccelByteWarsActivatableWidget::UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAccelByteWarsActivatableWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Refresh Tutorial Module metadatas based on the default object.
	const UAccelByteWarsActivatableWidget* DefaultObj = Cast<UAccelByteWarsActivatableWidget>(GetClass()->GetDefaultObject());
	if (DefaultObj)
	{
		AssociateTutorialModule = DefaultObj->AssociateTutorialModule;

		GeneratedWidgets = DefaultObj->GeneratedWidgets;
		FTUEDialogues = DefaultObj->FTUEDialogues;
	}
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
	InitializeFTEUDialogues(bOnActivatedInitializeFTUE);
}

void UAccelByteWarsActivatableWidget::NativeOnDeactivated()
{
	DeinitializeFTUEDialogues();

	Super::NativeOnDeactivated();
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

void UAccelByteWarsActivatableWidget::PostLoad()
{
	Super::PostLoad();

	ValidateAssociateTutorialModule();
	ValidateGeneratedWidgets();
	ValidateFTUEDialogues();
}

#if WITH_EDITOR
void UAccelByteWarsActivatableWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ValidateAssociateTutorialModule();
	ValidateGeneratedWidgets();
	ValidateFTUEDialogues();
}

void UAccelByteWarsActivatableWidget::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	ValidateAssociateTutorialModule();
	ValidateGeneratedWidgets();
	ValidateFTUEDialogues();
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
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot move the camera to active menu. Camera is not found."));
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
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to change input mode. Player controller is not valid or already destroyed."));
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
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to change input mode. Player controller is not valid or already destroyed."));
		return;
	}

	// Set input mode to game only mode.
	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = false;
}

#pragma region "Tutorial Module"
void UAccelByteWarsActivatableWidget::ValidateAssociateTutorialModule()
{
	if (AssociateTutorialModule && AssociateTutorialModule->GetTutorialModuleUIClass() != GetClass())
	{
		AssociateTutorialModule = nullptr;
	}
}
#pragma endregion

#pragma region "Generated Widgets"
void UAccelByteWarsActivatableWidget::ValidateGeneratedWidgets()
{
	GeneratedWidgets.RemoveAll([](const FTutorialModuleGeneratedWidget* Temp)
	{
		return !Temp || !Temp->OwnerTutorialModule;
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
		if (!OldGeneratedWidget->IsUnreachable()) 
		{
			OldGeneratedWidget->ConditionalBeginDestroy();
		}
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
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Log, TEXT("Tutorial Module Data Asset is not active. Cannot initialize the generated widget."));
			continue;
		}

		// Get valid widget container.
		if (!ensure(GetGeneratedWidgetContainers().IsValidIndex(GeneratedWidget->TargetWidgetContainerIndex)))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module widget's Target Widget Container index is out of bound. Cannot initialize the widget."));
			continue;
		}
		UPanelWidget* WidgetContainer = GetGeneratedWidgetContainers()[GeneratedWidget->TargetWidgetContainerIndex];
		if (!ensure(WidgetContainer))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module widget's Target Widget Container is null. Cannot initialize the widget."));
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
	
		GeneratedWidget->OnWidgetGenerated.Broadcast();
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
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
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

	// Rename the button based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		Button->Rename(*Metadata.WidgetId);
	}

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
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module's Button Action with id {%s} is clicked but doesn't have any action."), *Metadata.WidgetId);
			return;
		}

		Metadata.ButtonAction.Broadcast();
	});
	WidgetContainer.AddChild(Button.Get());
	Metadata.GenerateWidgetRef = Button.Get();

	// Rename the button based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		Button->Rename(*Metadata.WidgetId);
	}

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
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Widget class is null. Cannot initialize the generated the Tutorial Module's widget."));
		return nullptr;
	}

	// Spawn the widget.
	const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, WidgetClass.Get()));
	WidgetContainer.AddChild(Widget.Get());
	Metadata.GenerateWidgetRef = Widget.Get();

	// Rename the widget based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		Widget->Rename(*Metadata.WidgetId);
	}

	GeneratedWidgetPool.Add(Widget.Get());

	return Widget;
}
#pragma endregion

#pragma region "First Time User Experience (FTUE)"

void UAccelByteWarsActivatableWidget::ValidateFTUEDialogues()
{
	// Clear invalid FTUE dialogues.
	FTUEDialogues.RemoveAll([](const FFTUEDialogueModel* Temp)
	{
		return !Temp || !Temp->OwnerTutorialModule;
	});
}

void UAccelByteWarsActivatableWidget::InitializeFTEUDialogues(bool bShowOnInitialize)
{
	if (!IsActivated() || IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues as the widget begin to tear down."));
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. Game Instance is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. Base UI widget is not valid."));
		return;
	}

	UFTUEDialogueWidget* FTUEWidget = BaseUIWidget->GetFTUEDialogueWidget();
	if (!FTUEWidget) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. FTUE dialogue widget is not valid."));
		return;
	}

	// Assign FTUE dialogues.
	FTUEWidget->AddDialogues(FTUEDialogues);
	if (!bShowOnInitialize) 
	{
		return;
	}

	// Try to auto show the FTUE dialogues.
	FTUEWidget->ShowDialoguesFirstTime();

	// If the activated widget is in prompt stack, try to pause FTUE dialogues.
	if (auto* Widget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Prompt, this))
	{
		if (Widget->IsActivated())
		{
			FTUEWidget->CloseDialogues();
			FTUEWidget->TryToggleHelpDev(false);
		}
	}
}

void UAccelByteWarsActivatableWidget::DeinitializeFTUEDialogues()
{
	if (IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize FTUE dialogues as the widget begin to tear down."));
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize FTUE dialogues. Game Instance is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget(false);
	if (!BaseUIWidget)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize FTUE dialogues. Base UI widget is not valid."));
		return;
	}

	UFTUEDialogueWidget* FTUEWidget = BaseUIWidget->GetFTUEDialogueWidget();
	if (!FTUEWidget)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize FTUE dialogues. FTUE dialogue widget is not valid."));
		return;
	}

	// Only close the FTUE dialogue if this widget ever added dialogues to it.
	if (FTUEWidget->RemoveAssociateDialogues(GetClass())) 
	{
		FTUEWidget->CloseDialogues();
	}

	// If the deactivated widget is in prompt stack, try to resume FTUE dialogues.
	if (auto* Widget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Prompt, this))
	{
		if (!Widget->IsActivated()) 
		{
			FTUEWidget->ShowDialoguesFirstTime();
		}
	}
}

#pragma endregion

#undef LOCTEXT_NAMESPACE