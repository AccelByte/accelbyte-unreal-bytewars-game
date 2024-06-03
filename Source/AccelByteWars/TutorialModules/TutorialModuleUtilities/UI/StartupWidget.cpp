// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "StartupWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"

#include "Kismet/GameplayStatics.h"
#include "Components/WidgetSwitcher.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

bool UStartupWidget::bIsInitialized = false;

void UStartupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance())) 
	{
		BaseUIWidget = GameInstance->GetBaseUIWidget();
		PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	}
}

void UStartupWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_QuitGame->OnClicked().AddUObject(this, &ThisClass::QuitGame);

	// Show initializing spinner and delay it if not already initialized.
	if (!bIsInitialized) 
	{
		bIsInitialized = true;
		Ws_Startup->SetActiveWidget(Pw_Init);
		GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &ThisClass::StartupGame, InitDelay, false);
	}
	else 
	{
		Pw_Init->SetVisibility(ESlateVisibility::Collapsed);
		StartupGame();
	}
}

void UStartupWidget::NativeOnDeactivated()
{
	Btn_QuitGame->OnClicked().Clear();
	GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);

	Super::NativeOnDeactivated();
}

void UStartupWidget::StartupGame()
{
	GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);

	// Check whether the sdk configuration is valid.
	if (!UTutorialModuleOnlineUtility::IsAccelByteSDKInitialized(this)) 
	{
		Tb_FailedMessage->SetText(LOCTEXT("SDK Init Failed", "AccelByte SDK is not initialized. Please enable it and set up your credentials."));
		Ws_Startup->SetActiveWidget(Pw_Failed);
		return;
	}

	if (!BaseUIWidget)
	{
		Tb_FailedMessage->SetText(LOCTEXT("Start Up Failed", "Start up failed. BaseUIWidget is empty."));
		Ws_Startup->SetActiveWidget(Pw_Failed);
	}

	// If auth essentials module is not active, open the Main Menu.
	if ((!AuthEssentialsDataAsset || !AuthEssentialsDataAsset->IsActiveAndDependenciesChecked()) && BaseUIWidget)
	{
		DeactivateWidget();
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, MainMenuWidgetClass.Get());
		return;
	}

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		Tb_FailedMessage->SetText(LOCTEXT("Start Up Failed", "Start up failed. Failed to get Subsystem."));
		Ws_Startup->SetActiveWidget(Pw_Failed);
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface)
	{
		Tb_FailedMessage->SetText(LOCTEXT("Start Up Failed", "Start up failed. Failed to get IdentityInterface."));
		Ws_Startup->SetActiveWidget(Pw_Failed);
		return;
	}

	// If already login, immediately open the Main Menu.
	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::Type::LoggedIn && BaseUIWidget)
	{
		DeactivateWidget();
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, MainMenuWidgetClass.Get());
		return;
	}

	const TSubclassOf<UAccelByteWarsActivatableWidget> LoginWidgetClass = AuthEssentialsDataAsset->GetTutorialModuleUIClass();
	if (!LoginWidgetClass)
	{
		Tb_FailedMessage->SetText(LOCTEXT("Start Up Failed", "Start up failed. LoginWidgetClass is not set."));
		Ws_Startup->SetActiveWidget(Pw_Failed);
		return;
	}

	// If not logged in, open the login menu.
	DeactivateWidget();
	if(BaseUIWidget) BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, LoginWidgetClass.Get());

	IdentityInterface->AccelByteOnLogoutCompleteDelegates->RemoveAll(this);
	IdentityInterface->AccelByteOnLogoutCompleteDelegates->AddUObject(this, &ThisClass::OnLogoutComplete);
}

void UStartupWidget::QuitGame()
{
	if (!ensure(PromptSubsystem)) 
	{
		return;
	}

	PromptSubsystem->ShowDialoguePopUp(
		LOCTEXT("Quit Game", "Quit Game"),
		LOCTEXT("Are you sure?", "Are you sure?"),
		EPopUpType::ConfirmationYesNo,
		FPopUpResultDelegate::CreateWeakLambda(this, [this](EPopUpResult Result)
		{
			if (Result == EPopUpResult::Confirmed)
			{
				FPlatformMisc::RequestExit(false);
			}
		})
	);
}

void UStartupWidget::OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error)
{
	if (BaseUIWidget)
	{
		if (TSubclassOf<UAccelByteWarsActivatableWidget> LoginWidgetClass = AuthEssentialsDataAsset->GetTutorialModuleUIClass())
		{
			DeactivateWidget();
			BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, LoginWidgetClass.Get());
		}
	}
}

#undef LOCTEXT_NAMESPACE