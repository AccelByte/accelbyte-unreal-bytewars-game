// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
 
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Editor/WidgetCompilerLog.h"
#include "CommonInputModeTypes.h"
#include "Input/CommonUIInputTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/AccelByteWarsTabListWidget.h"

#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/OverlaySlot.h"

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsActivatableWidget);

#define LOCTEXT_NAMESPACE "AccelByteWars"
#define BTN_DEV_HELP_NAME TEXT("Btn_DevHelp")

UAccelByteWarsActivatableWidget::UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAccelByteWarsActivatableWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance()))
	{
		// Assign triggerer to open AccelByte SDK config menu.
		OpenSdkConfigHandle.Unregister();
		OpenSdkConfigHandle = RegisterUIActionBinding(FBindUIActionArgs(
			GameInstance->GetOpenSDKConfigMenuInputAction(),
			false,
			FSimpleDelegate::CreateUObject(GameInstance, &UAccelByteWarsGameInstance::OpenSDKConfigMenu)));

#pragma region "Lobby Connect/Disconnect using PS Controller"
#ifdef AGS_LOBBY_CHEAT_ENABLED
		if (AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_CHEAT_LOBBY, SECTION_CHEAT_LOBBY, false))
		{
			LobbyConnectHandle.Unregister();
			LobbyConnectHandle = RegisterUIActionBinding(FBindUIActionArgs(
				GameInstance->LobbyConnectInputActionData,
				false,
				FSimpleDelegate::CreateUObject(GameInstance, &UAccelByteWarsGameInstance::LobbyConnect)));

			LobbyDisconnectHandle.Unregister();
			LobbyDisconnectHandle = RegisterUIActionBinding(FBindUIActionArgs(
				GameInstance->LobbyDisconnectInputActionData,
				false,
				FSimpleDelegate::CreateUObject(GameInstance, &UAccelByteWarsGameInstance::LobbyDisconnect)));
		}
#endif // AGS_LOBBY_CHEAT_ENABLED
#pragma endregion 
	}

	// Refresh Tutorial Module metadatas based on the default object.
	const UAccelByteWarsActivatableWidget* DefaultObj = Cast<UAccelByteWarsActivatableWidget>(GetClass()->GetDefaultObject());
	if (DefaultObj)
	{
		AssociateTutorialModule = DefaultObj->AssociateTutorialModule;

		GeneratedWidgets = DefaultObj->GeneratedWidgets;
		WidgetValidators = DefaultObj->WidgetValidators;
		GUICheatWidgetEntries = DefaultObj->GUICheatWidgetEntries;
		FTUEDialogues = DefaultObj->FTUEDialogues;
	}
}

void UAccelByteWarsActivatableWidget::NativeOnActivated()
{
	InitializeGeneratedWidgets();
	InitializeGUICheatWidgetEntries();

	// Execute on-activated event for default object.
	if (GetClass())
	{
		if (const UAccelByteWarsActivatableWidget* DefaultObj = Cast<UAccelByteWarsActivatableWidget>(GetClass()->GetDefaultObject()))
		{
			DefaultObj->OnActivated().Broadcast();
		}
	}

	Super::NativeOnActivated();

	// Set visible only if the associate Tutorial Module is active.
	if (AssociateTutorialModule)
	{
		const bool bIsTutorialModuleActive = AssociateTutorialModule->IsActiveAndDependenciesChecked();
		SetVisibility(bIsTutorialModuleActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	ValidateDevHelpButton();
	ExecuteWidgetValidators();
	if(bLoadFTUEImmediately)
	{
		InitializeFTUEDialogues(bOnActivatedInitializeFTUE);
	}

	SetVisibility(GetIsAllGeneratedWidgetsShouldNotDisplay() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	RequestRefreshFocus();

	// Tell the tab list that the generated widget is done constructing
	for (UAccelByteWarsTabListWidget* WidgetTabList : GetGeneratedWidgetTabListContainers())
	{
		if (!WidgetTabList)
		{
			continue;
		}

		WidgetTabList->ParentOnActivated();
	}
}

void UAccelByteWarsActivatableWidget::NativeOnDeactivated()
{
	DeinitializeFTUEDialogues();
	DeInitializeGUICheatWidgetEntries();

	// Execute on-deactivated event for default object.
	if (GetClass()) 
	{
		if (const UAccelByteWarsActivatableWidget* DefaultObj = Cast<UAccelByteWarsActivatableWidget>(GetClass()->GetDefaultObject()))
		{
			DefaultObj->OnDeactivated().Broadcast();
		}
	}

	Super::NativeOnDeactivated();

	// Tell the tab list that the generated widget is done deactivating
	for (UAccelByteWarsTabListWidget* WidgetTabList : GetGeneratedWidgetTabListContainers())
	{
		if (!WidgetTabList)
		{
			continue;
		}

		WidgetTabList->ParentOnDeactivated();
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

void UAccelByteWarsActivatableWidget::SetWidgetStackType(const EBaseUIStackType StackType)
{
	WidgetStackType = StackType;
}

EBaseUIStackType UAccelByteWarsActivatableWidget::GetWidgetStackType() const
{
	return WidgetStackType;
}

void UAccelByteWarsActivatableWidget::AllowMoveCamera(bool bAllow)
{
	bAllowMoveCamera = bAllow;
}

void UAccelByteWarsActivatableWidget::PostLoad()
{
	Super::PostLoad();

	ValidateAssociateTutorialModule();
}

#if WITH_EDITOR
void UAccelByteWarsActivatableWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ValidateAssociateTutorialModule();
}

void UAccelByteWarsActivatableWidget::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	ValidateAssociateTutorialModule();
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

bool UAccelByteWarsActivatableWidget::GetIsAllGeneratedWidgetsShouldNotDisplay() const
{
	bool bAllShouldNotDisplay = false;
	if (bHideIfGeneratedWidgetEmpty)
	{
		bAllShouldNotDisplay = true;
		for (const FTutorialModuleGeneratedWidget* GeneratedWidget : GeneratedWidgets)
		{
			if (!GeneratedWidget)
			{
				continue;
			}

			if (GetValidEntryWidgetClass(*GeneratedWidget))
			{
				bAllShouldNotDisplay = false;
				break;
			}
		}
	}
	
	return bAllShouldNotDisplay;
}

void UAccelByteWarsActivatableWidget::MoveCameraToTargetLocation(const float DeltaTime, const FVector TargetLocation, const float InterpSpeed)
{
	if (!bAllowMoveCamera)
	{
		return;
	}

	AActor* Camera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
	if (Camera == nullptr) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot move the camera to target location. Camera is not found."));
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

	// Validate tutorial module metadatas
	ValidateGeneratedWidgets();
	ValidateWidgetValidators();
	ValidateFTUEDialogues();
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
		if (GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST ||
			GeneratedWidget->WidgetType == ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST)
		{
			// tab list has its own function to add entries
			if (!ensure(GetGeneratedWidgetTabListContainers().IsValidIndex(GeneratedWidget->TargetWidgetContainerIndex)))
			{
				UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module widget's Target Tab List Widget Container index is out of bound. Cannot initialize the widget."));
				continue;
			}
			UAccelByteWarsTabListWidget* WidgetContainer = GetGeneratedWidgetTabListContainers()[GeneratedWidget->TargetWidgetContainerIndex];
			if (!ensure(WidgetContainer))
			{
				UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module widget's Target Tab List Widget Container is null. Cannot initialize the widget."));
				continue;
			}
			if (!ensure(WidgetContainer->GetUsePresetButtonClass()))
			{
				UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module widget's Target Tab List Widget Container's bUsePresetButtonClass is null. Cannot initialize the widget."));
				continue;
			}

			GenerateTabListEntryButton(*GeneratedWidget, *WidgetContainer);
		}
		else
		{
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
		}

		if (GeneratedWidget)
		{
			GeneratedWidget->OnWidgetGenerated.Broadcast();
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

	TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = GetValidEntryWidgetClass(Metadata);
	if (!EntryWidgetClass)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
		return nullptr;
	}

	// Spawn the entry button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
	if (!Button.Get())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to create an instance of widget entry button"));
		return nullptr;
	}

	// Rename the button based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		if (!Button->Rename(*Metadata.WidgetId))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to rename widget entry button %s to %s"), *Button->GetName(), *Metadata.WidgetId);
		}
	}

	// Spawn button.
	UPanelSlot* ButtonSlot = WidgetContainer.AddChild(Button.Get());
	if (!ButtonSlot)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to add widget entry button %s to the target container."), *Button->GetName());
		Button->Destruct();
		return nullptr;
	}

	// Assign action to open widget when entry button is clicked.
	if (Metadata.ButtonType == ETutorialModuleButtonType::TEXT_BUTTON) 
	{
		Button->SetButtonType(EAccelByteWarsButtonBaseType::TEXT_BUTTON);
		Button->SetButtonText(Metadata.ButtonText);
	}
	else 
	{
		Button->SetButtonType(EAccelByteWarsButtonBaseType::IMAGE_BUTTON);
		Button->SetButtonImage(Metadata.ButtonImage);
	}
	if (!Metadata.ButtonInputActionData.IsNull()) 
	{
		Button->SetTriggeringInputAction(Metadata.ButtonInputActionData);
	}
	Button->OnClicked().AddWeakLambda(this, [Metadata, BaseUIWidget, EntryWidgetClass]()
	{
		// Check if action is valid.
		if (Metadata.ValidateButtonAction.IsBound() && !Metadata.ValidateButtonAction.Execute())
		{
			return;
		}

		// Execute action.
		BaseUIWidget->PushWidgetToStack(Metadata.TargetStackToSpawn, EntryWidgetClass);
	});
	Metadata.GenerateWidgetRef = Button.Get();

	// Refresh button alignment since the default alignment upon a widget is spawned is "Align_Fill".
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

	// Create action button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DefaultButtonClass.Get()));
	if (!Button.Get())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to create an instance of action button"));
		return nullptr;
	}

	// Rename the button based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		if (!Button->Rename(*Metadata.WidgetId))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to rename action button %s to %s"), *Button->GetName(), *Metadata.WidgetId);
		}
	}

	// Spawn button.
	UPanelSlot* ButtonSlot = WidgetContainer.AddChild(Button.Get());
	if (!ButtonSlot)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to add action button %s to the target container."), *Button->GetName());
		Button->Destruct();
		return nullptr;
	}

	// Assign action.
	if (Metadata.ButtonType == ETutorialModuleButtonType::TEXT_BUTTON)
	{
		Button->SetButtonType(EAccelByteWarsButtonBaseType::TEXT_BUTTON);
		Button->SetButtonText(Metadata.ButtonText);
	}
	else
	{
		Button->SetButtonType(EAccelByteWarsButtonBaseType::IMAGE_BUTTON);
		Button->SetButtonImage(Metadata.ButtonImage);
	}
	Button->OnClicked().AddWeakLambda(this, [&Metadata]()
	{
		// Check if action is valid.
		if (Metadata.ValidateButtonAction.IsBound() && !Metadata.ValidateButtonAction.Execute()) 
		{
			return;
		}

		// Execute action.
		if (!Metadata.ButtonAction.IsBound())
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Tutorial Module's Button Action with id {%s} is clicked but doesn't have any action."), *Metadata.WidgetId);
			return;
		}

		Metadata.ButtonAction.Broadcast();
	});
	Metadata.GenerateWidgetRef = Button.Get();

	// Refresh button alignment since the default alignment upon a widget is spawned is "Align_Fill".
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
	const UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass = GetValidEntryWidgetClass(Metadata);
	if (!WidgetClass)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Widget class is null. Cannot initialize the generated Tutorial Module's widget."));
		return nullptr;
	}

	// Create the widget.
	const TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = MakeWeakObjectPtr<UAccelByteWarsActivatableWidget>(CreateWidget<UAccelByteWarsActivatableWidget>(this, WidgetClass.Get()));
	if (!Widget.Get()) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to create an instance of generated widget with type %s"), *WidgetClass->GetName());
		return nullptr;
	}

	// Rename the widget based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		if (!Widget->Rename(*Metadata.WidgetId))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to rename generated widget %s to %s"), *Widget->GetName(), *Metadata.WidgetId);
		}
	}

	// Spawn widget.
	if (!WidgetContainer.AddChild(Widget.Get()))
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to add generated widget %s to the target container."), *Widget->GetName());
		Widget->Destruct();
		return nullptr;
	}

	// Activate widget.
	Widget->ActivateWidget();
	Metadata.GenerateWidgetRef = Widget.Get();

	GeneratedWidgetPool.Add(Widget.Get());

	return Widget;
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GenerateTabListEntryButton(
	FTutorialModuleGeneratedWidget& Metadata,
	UAccelByteWarsTabListWidget& WidgetContainer)
{
	TWeakObjectPtr<UAccelByteWarsButtonBase> Button =
		Cast<UAccelByteWarsButtonBase>(WidgetContainer.GetTabButtonBaseByID(Metadata.TabNameId));

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);
	const UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);
	const TSubclassOf<UAccelByteWarsButtonBase> DefaultButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DefaultButtonClass.Get());

	const TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = GetValidEntryWidgetClass(Metadata);
	if (!EntryWidgetClass)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(
			Warning,
			TEXT("Entry widget class is null. Cannot initialize the generated entry button."));
		return nullptr;
	}

	// Create entry widget
	UAccelByteWarsActivatableWidget* EntryWidget = CreateWidget<UAccelByteWarsActivatableWidget>(this, EntryWidgetClass.Get());

	// If the tab list is already registered, update the content widget instead.
	bool bRegistered = false;
	if (Button.IsValid())
	{
		bRegistered = WidgetContainer.RegisterTabContentWidget(Metadata.TabNameId, EntryWidget);
	}
	// Otherwise, register a new tab list entry
	else 
	{
		bRegistered = WidgetContainer.RegisterTabWithPresets(Metadata.TabNameId, Metadata.ButtonText, EntryWidget, Metadata.TabIndex);
		Button = Cast<UAccelByteWarsButtonBase>(WidgetContainer.GetTabButtonBaseByID(Metadata.TabNameId));
	}

	// Validate new tab list registration.
	if (!bRegistered)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(
			Warning,
			TEXT("Failed to register tab to target widget's tab list container. Check log prior to this log."));
		return nullptr;
	}

	// Validate the tab list entry button.
	if (!Button.Get())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to create an instance of widget entry button"));
		return nullptr;
	}

	// Rename the button based on its id.
	if (!Metadata.WidgetId.IsEmpty())
	{
		if (!Button->Rename(*Metadata.WidgetId))
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to rename widget entry button %s to %s"), *Button->GetName(), *Metadata.WidgetId);
		}
	}

	EntryWidget->ActivateWidget();
	Metadata.GenerateWidgetRef = Button.Get();

	return Button;
}

TSubclassOf<UAccelByteWarsActivatableWidget> UAccelByteWarsActivatableWidget::GetValidEntryWidgetClass(
	const FTutorialModuleGeneratedWidget& Metadata) const
{
	UTutorialModuleDataAsset* SourceTutorialModule = nullptr;
	TSubclassOf<UAccelByteWarsActivatableWidget> EntryWidgetClass = nullptr;
	switch (Metadata.WidgetType)
	{
	case ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON:
	case ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET:
	case ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST:
		SourceTutorialModule = Metadata.OwnerTutorialModule;
		break;
	case ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON:
	case ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET:
	case ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST:
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
		return nullptr;
	}

	// If the target widget have bHideIfGeneratedWidgetEmpty true and its generated widget is empty
	if (const UAccelByteWarsActivatableWidget* DefaultObject = Cast<UAccelByteWarsActivatableWidget>(EntryWidgetClass->GetDefaultObject()))
	{
		if (DefaultObject->GetIsAllGeneratedWidgetsShouldNotDisplay())
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Log, TEXT("Widget class (%s) have its bHideIfGeneratedWidgetEmpty set to true and all of its GeneratedWidgets is empty / not visible. Skipping generation"), *DefaultObject->GetName())
			return nullptr;
		}
	}

	return EntryWidgetClass;
}
#pragma endregion

#pragma region "First Time User Experience (FTUE)"

void UAccelByteWarsActivatableWidget::OpenFTUEWidget() const
{
	const TWeakObjectPtr<UAccelByteWarsButtonBase> DevHelpButton = GetDevHelpButton();
	if (DevHelpButton == nullptr)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. This should not be called since there is no DevHelp Button."));
		return;
	}

	if (!IsActivated() || IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues as the widget begins to tear down."));
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

	if (FTUEDialogues.IsEmpty())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. Widget didn't have FTUE dialogues."));
		return;
	}

	UAccelByteWarsActivatableWidget* FTUEWidgetInstance = BaseUIWidget->PushWidgetToStack(EBaseUIStackType::FTUE, GameInstance->GetFTUEWidgetClass().Get());

	UFTUEDialogueWidget* FTUEWidget = StaticCast<UFTUEDialogueWidget*>(FTUEWidgetInstance);

	BaseUIWidget->SetFTUEDialogueWidget(FTUEWidget);

	// Assign FTUE dialogues.
	FTUEWidget->AddDialogues(FTUEDialogues);

	// Try to auto show the FTUE dialogues.
	FTUEWidget->ShowDialogues(false);
	HideFTUEDevHelpInputAction();
}

void UAccelByteWarsActivatableWidget::HideFTUEDevHelpButton(const bool bInHideButton) const
{
	const TWeakObjectPtr<UAccelByteWarsButtonBase> DevHelpButton = GetDevHelpButton();
	if (DevHelpButton == nullptr)
	{
		return;
	}

	DevHelpButton->SetVisibility(bInHideButton ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UAccelByteWarsActivatableWidget::HideFTUEDevHelpInputAction(const bool bInHideInputAction) const
{
	const TWeakObjectPtr<UAccelByteWarsButtonBase> DevHelpButton = GetDevHelpButton();
	if (!DevHelpButton.IsValid())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot hide FTUE dev help input action. The Dev Help button is null."));
		return;
	}

	const bool bIsTearingDown = GetWorld() && GetWorld()->bIsTearingDown;
	if (bIsTearingDown)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot hide FTUE dev help input action as the UWorld is tearing down."));
		return;
	}

	DevHelpButton->SetHideInputAction(bInHideInputAction);
}

void UAccelByteWarsActivatableWidget::InitializeFTUEDialogues(const bool bShowOnInitialize, const bool bIsFirstTime) const
{
	if (!IsActivated() || IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues as the widget begins to tear down."));
		return;
	}

	if (FTUEDialogues.IsEmpty())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. Widget didn't have FTUE dialogues."));
		return;
	}

	if (!bShowOnInitialize)
	{
		return;
	}

	// Create FTUE widget only after validation finished, to avoid input blocked if validation take a long time  
	CreateFTUEWidgetDelegate.Unbind();
	CreateFTUEWidgetDelegate.BindWeakLambda(this, [this, bIsFirstTime](const bool bIsInterrupted) mutable
	{
		if(bIsInterrupted)
		{
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
		if (IsValid(FTUEWidget)) 
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. FTUE dialogue widget is already initialized."));
			return;
		}
		
		UAccelByteWarsActivatableWidget* FTUEWidgetInstance = BaseUIWidget->PushWidgetToStack(EBaseUIStackType::FTUE, GameInstance->GetFTUEWidgetClass().Get());
		if(FTUEWidgetInstance == nullptr)
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot initialize FTUE dialogues. Failed to push widget to stack."));
			return;
		}

		FTUEWidget = StaticCast<UFTUEDialogueWidget*>(FTUEWidgetInstance);
		BaseUIWidget->SetFTUEDialogueWidget(FTUEWidget);

		// Assign FTUE dialogues.
		if(AccelByteWarsUtility::IsUseVersionChecker())
		{
			FTUEWidget->AddDialogues(FTUEDialogues);
		}
		else
		{
			// Remove check version dialog if check version setting is off
			TArray<FFTUEDialogueModel*> Dialogues = FTUEDialogues.FilterByPredicate([](const FFTUEDialogueModel* Dialogue)
			{
				return Dialogue->Validator.ValidatorType != EServicePredefinedValidator::IS_VALID_CONFIG_VERSION;
			});
			FTUEWidget->AddDialogues(Dialogues);
		}

		// If prompt is active, pause the FTUE.
		const EBaseUIStackType TopMostActiveStack = BaseUIWidget->GetTopMostActiveStack();
		if (TopMostActiveStack == EBaseUIStackType::Prompt) 
		{
			const UCommonActivatableWidget* PromptWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(TopMostActiveStack, this);
			if (PromptWidget && PromptWidget->IsActivated())
			{
				FTUEWidget->PauseDialogues();
				return;
			}
		}
		// Deinitialize FTUE for other widgets below the current active stack.
		else 
		{
			for (UCommonActivatableWidget* WidgetBelow : BaseUIWidget->GetAllWidgetsBelowStacks(EBaseUIStackType::Menu))
			{
				UAccelByteWarsActivatableWidget* OtherWidget = Cast<UAccelByteWarsActivatableWidget>(WidgetBelow);
				if (!OtherWidget || OtherWidget == this)
				{
					continue;
				}

				OtherWidget->DeinitializeFTUEDialogues();
			}
		}

		// Try to auto show the FTUE dialogues.
		FTUEWidget->ShowDialogues(bIsFirstTime);
		HideFTUEDevHelpInputAction(bIsFirstTime);
	});
	
	FFTUEDialogueModel::ValidateDialogues(FTUEDialogues, this, CreateFTUEWidgetDelegate);
}

void UAccelByteWarsActivatableWidget::DeinitializeFTUEDialogues() const
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

	// Cancel ongoing FTUE validation (if any) for this widget
	FFTUEDialogueModel::TryInterruptValidation(this);
	
	UFTUEDialogueWidget* FTUEWidget = BaseUIWidget->GetFTUEDialogueWidget();
	if (!IsValid(FTUEWidget))
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize FTUE dialogues. FTUE dialogue widget is not valid."));
		return;
	}

	// Only close the FTUE dialogue if this widget ever added dialogues to it.
	if (FTUEWidget->RemoveAssociateDialogues(GetClass())) 
	{
		FTUEWidget->CloseDialogues();
		BaseUIWidget->SetFTUEDialogueWidget(nullptr);
	}

	// If prompt is deactivating, resume the dialogue.
	const EBaseUIStackType TopMostActiveStack = BaseUIWidget->GetTopMostActiveStack();
	if (TopMostActiveStack == EBaseUIStackType::Prompt)
	{
		const UCommonActivatableWidget* PromptWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(TopMostActiveStack, this);
		if (!PromptWidget || !PromptWidget->IsActivated())
		{
			FTUEWidget->ResumeDialogues();
		}

	}
	// Initialize FTUE for other widgets below the current active stack.
	else
	{
		for (UCommonActivatableWidget* WidgetBelow : BaseUIWidget->GetAllWidgetsBelowStacks(EBaseUIStackType::Menu))
		{
			const UAccelByteWarsActivatableWidget* OtherWidget = Cast<UAccelByteWarsActivatableWidget>(WidgetBelow);
			if (!OtherWidget || OtherWidget == this)
			{
				continue;
			}

			OtherWidget->InitializeFTUEDialogues(true);
		}
	}
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GenerateDevHelpButton()
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	const TSubclassOf<UAccelByteWarsButtonBase> DevHelpButtonClass = GameInstance->GetDefaultButtonClass();
	ensure(DevHelpButtonClass.Get());

	// Spawn the entry button.
	const TWeakObjectPtr<UAccelByteWarsButtonBase> Button = MakeWeakObjectPtr<UAccelByteWarsButtonBase>(CreateWidget<UAccelByteWarsButtonBase>(this, DevHelpButtonClass.Get()));
	if (!Button.Get())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to create an instance of widget entry button"));
		return nullptr;
	}

	if (!Button->Rename(BTN_DEV_HELP_NAME))
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Failed to rename widget entry button %s to %s"), *Button->GetName(), BTN_DEV_HELP_NAME);
	}

	const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (ParentWidget == nullptr)
	{
		const EBaseUIStackType TopMostActiveStack = BaseUIWidget->GetTopMostActiveStack();
		ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(TopMostActiveStack, this);
	}

	if (!ParentWidget->IsActivated())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot add widget entry button %s to the non active container."), *Button->GetName());
		return nullptr;
	}

	UPanelWidget* Root = Cast<UPanelWidget>(ParentWidget->GetRootWidget());
	Root->AddChild(Button.Get());

	Button->SetButtonType(EAccelByteWarsButtonBaseType::TEXT_BUTTON);
	Button->SetButtonText(LOCTEXT("Dev Help", "Dev Help"));
	Button->SetTriggeringInputAction(GameInstance->GetDevHelpButtonInputAction());
	Button->SetStyle(GameInstance->GetDevHelpButtonStyle());
	Button->SetHideInputAction(false);
	Button->SetIsFocusable(false);
	Button->OnClicked().AddWeakLambda(this, [this] { OpenFTUEWidget(); });

	const FMargin ButtonMargin = FMargin(0, 0, 30, 30);
	if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Button->Slot))
	{
		OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		OverlaySlot->SetPadding(ButtonMargin);
	}
	else if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(Button->Slot))
	{
		VerticalSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		VerticalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		VerticalSlot->SetPadding(ButtonMargin);
	}
	else if (UHorizontalBoxSlot* HorizontalSlot = Cast<UHorizontalBoxSlot>(Button->Slot))
	{
		HorizontalSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		HorizontalSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		HorizontalSlot->SetPadding(ButtonMargin);
	}

	GeneratedWidgetPool.Add(Button.Get());

	return Button;
}

TWeakObjectPtr<UAccelByteWarsButtonBase> UAccelByteWarsActivatableWidget::GetDevHelpButton() const
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UAccelByteWarsButtonBase::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (!Widget->GetName().Equals(BTN_DEV_HELP_NAME))
		{
			continue;
		}

		UAccelByteWarsButtonBase* DevHelpButton = StaticCast<UAccelByteWarsButtonBase*>(Widget);
		return MakeWeakObjectPtr<UAccelByteWarsButtonBase>(DevHelpButton);
	}

	return nullptr;
}

void UAccelByteWarsActivatableWidget::DestroyDevHelpButton() const
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UAccelByteWarsButtonBase::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (!Widget->GetName().Equals(BTN_DEV_HELP_NAME))
		{
			continue;
		}

		Widget->RemoveFromParent();
		Widget->ConditionalBeginDestroy();
		return;
	}
}

void UAccelByteWarsActivatableWidget::ValidateFTUEDialogues()
{
	// Clear invalid FTUE dialogues.
	FTUEDialogues.RemoveAll([](const FFTUEDialogueModel* Temp)
	{
		return !Temp || !Temp->OwnerTutorialModule;
	});
}

void UAccelByteWarsActivatableWidget::ValidateDevHelpButton()
{
	if (FTUEDialogues.IsEmpty())
	{
		return;
	}

	FTUEDialogues[0]->ExecuteValidation(FOnFTUEDialogueValidationComplete::CreateUObject(this, &ThisClass::OnValidateDialogueComplete), this);
}

void UAccelByteWarsActivatableWidget::OnValidateDialogueComplete(FFTUEDialogueModel* Dialogue, const bool bIsValid)
{
	if (!bIsValid || !Dialogue)
	{
		return;
	}

	DestroyDevHelpButton();
	GenerateDevHelpButton();
}

#pragma endregion

void UAccelByteWarsActivatableWidget::ValidateWidgetValidators()
{
	// Clear invalid widget validators.
	WidgetValidators.RemoveAll([](const FWidgetValidator* Temp)
	{
		return !Temp || !Temp->OwnerTutorialModule;
	});
}

void UAccelByteWarsActivatableWidget::ExecuteWidgetValidators()
{
	for (FWidgetValidator* Validator : WidgetValidators) 
	{
		if (!Validator) 
		{
			continue;
		}

		// Look for widget to validate.
		const FString WidgetToValidateStr = Validator->TargetWidgetNameToValidate;

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, Validator->TargetWidgetClassToValidate.Get(), false);

		// Eliminate invalid found widgets.
		FoundWidgets.RemoveAll([WidgetToValidateStr](const UUserWidget* Temp)
		{
			return !Temp || !Temp->GetName().Equals(WidgetToValidateStr) || !Temp->IsVisible();
		});
		if (FoundWidgets.IsEmpty()) 
		{
			continue;
		}

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
			// Skip if invalid.
			if (!FoundWidget || !FoundWidget->IsVisible())
			{
				continue;
			}

			// If widget found, execute widget validator.
			if (FoundWidget->GetName().Equals(WidgetToValidateStr, ESearchCase::CaseSensitive))
			{
				if (IAccelByteWarsWidgetInterface* WidgetInterface = Cast<IAccelByteWarsWidgetInterface>(FoundWidget)) 
				{
					UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Log, TEXT("Validator executed for widget by name %s"), *WidgetToValidateStr);
					Validator->ExecuteValidator(WidgetInterface, this);
				}
				else 
				{
					UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot execute widget validator. Widget by name %s does not implement valid widget interface."), *WidgetToValidateStr);
				}
				
				break;
			}
		}
	}
}

#pragma region "GUI Cheat"
void UAccelByteWarsActivatableWidget::InitializeGUICheatWidgetEntries()
{
	if (!AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_GUI_CHEAT, FLAG_GUI_CHEAT_SECTION, true))
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget();
	if (!BaseUI)
	{
		return;
	}

	// Add entry
	for (UGUICheatWidgetEntry* Entry : GUICheatWidgetEntries)
	{
		if (!Entry)
		{
			UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("GUICheat widget entry is invalid"))
			continue;
		}

		BaseUI->AddGUICheatEntry(Entry);
	}
}

void UAccelByteWarsActivatableWidget::DeInitializeGUICheatWidgetEntries() const
{
	if (!AccelByteWarsUtility::GetFlagValueOrDefault(FLAG_GUI_CHEAT, FLAG_GUI_CHEAT_SECTION, true))
	{
		return;
	}

	if (IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as the widget begin to tear down."));
		return;
	}

	if (GetWorld()->bIsTearingDown)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as the world begin to tear down."));
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as Game Instance is invalid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget(false);
	if (!BaseUI)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot deinitialize GUI Cheat as Base UI is invalid."));
		return;
	}

	// Remove entries
	for (UGUICheatWidgetEntry* Entry : GUICheatWidgetEntries)
	{
		BaseUI->RemoveGUICheatEntry(Entry);
	}
}
#pragma endregion 

#undef LOCTEXT_NAMESPACE
#undef BTN_DEV_HELP_NAME
