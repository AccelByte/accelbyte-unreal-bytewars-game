// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "StartupWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"

#include "Kismet/GameplayStatics.h"
#include "Components/WidgetSwitcher.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"
#include "TutorialModuleUtilities/StartupLog.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

bool UStartupWidget::bIsInitialized = false;

void UStartupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance())) 
	{
		BaseUIWidget = GameInstance->GetBaseUIWidget();
		PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
		StartupSubsystem = GameInstance->GetSubsystem<UStartupSubsystem>();
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
		GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &ThisClass::CheckCoreComponents, InitDelay, false);
	}
	else 
	{
		Pw_Init->SetVisibility(ESlateVisibility::Collapsed);
		CheckCoreComponents();
	}
}

void UStartupWidget::NativeOnDeactivated()
{
	Btn_QuitGame->OnClicked().Clear();
	GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);

	Super::NativeOnDeactivated();
}

void UStartupWidget::CheckCoreComponents()
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

	CheckAutoUseTokenForABLogin();
}

void UStartupWidget::CheckAutoUseTokenForABLogin()
{
	if (!ensure(PromptSubsystem) || !ensure(StartupSubsystem))
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to check auto use token for AccelByte login. Invalid subsystems"));
		return;
	}
	
	// If auto consume platform token, startup the game immediately.
	if (IsLoggedIn() || StartupSubsystem->IsAutoUseTokenForABLogin())
	{
		StartupGame();
		return;
	}

	PromptSubsystem->ShowLoading(LOCTEXT("Login platform only initiation", "Logging in with the platform only"));

	// Login using platform only.
	StartupSubsystem->LoginPlatformOnly(GetOwningPlayer(), FOnLoginPlatformCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginPlatformOnlyComplete));
}

void UStartupWidget::StartupGame()
{
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
	if (IsLoggedIn() && BaseUIWidget)
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
	if (BaseUIWidget) BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, LoginWidgetClass.Get());

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
				UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
			}
		})
	);
}

bool UStartupWidget::IsLoggedIn()
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		return false;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface))
	{
		return false;
	}

	return IdentityInterface->GetLoginStatus(0) == ELoginStatus::Type::LoggedIn;
}

void UStartupWidget::OnLoginPlatformOnlyComplete(const bool bSucceeded, const FString& ErrorMessage)
{
	if (!ensure(PromptSubsystem) || !ensure(StartupSubsystem))
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to handle on login platform only. Invalid subsystems"));
		return;
	}

	PromptSubsystem->HideLoading();

	// If failed, show error message and button to quit the game.
	if (!bSucceeded)
	{
		FFormatOrderedArguments Args;
		Args.Add(FFormatArgumentValue(FText::FromString(ErrorMessage)));

		const FText FailedMessage = FText::Format(
			LOCTEXT(
				"Login platform only failed message",
				"Failed to login with platform only. Click Ok to quit the game. Error: {0}"), 
				Args);

		PromptSubsystem->ShowDialoguePopUp(
			ERROR_PROMPT_TEXT,
			FailedMessage,
			EPopUpType::MessageOk,
			FPopUpResultDelegate::CreateWeakLambda(this, [](EPopUpResult Result)
			{
				FPlatformMisc::RequestExit(false);
			}
		));
		return;
	}

	FOnlineAccountCredentials PlatformCredentials = StartupSubsystem->GetPlatformCredentials();

	// For display purpose, construct truncated token if necessary.
	const int32 TokenLengthLimit = 20;
	const FString TokenStr = FString::Printf(TEXT("%s%s"),
		*PlatformCredentials.Token.Left(TokenLengthLimit),
		PlatformCredentials.Token.Len() > TokenLengthLimit ? TEXT("...") : TEXT(""));

	FFormatOrderedArguments Args;
	Args.Add(FFormatArgumentValue(FText::FromString(PlatformCredentials.Type)));
	Args.Add(FFormatArgumentValue(FText::FromString(TokenStr)));

	const FText SuccessMessage = FText::Format(
		LOCTEXT(
			"Login platform only success message",
			"Success to login with platform only. Platform name: {0}. Platform token: {1}"),
			Args);

	// Show success message and startup game on message closed.
	PromptSubsystem->ShowDialoguePopUp(
		MESSAGE_PROMPT_TEXT,
		SuccessMessage,
		EPopUpType::MessageOk,
		FPopUpResultDelegate::CreateWeakLambda(this, [this](EPopUpResult Result)
		{
			StartupGame();
		}
	));
}

void UStartupWidget::OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error)
{
	if (GetGameInstance())
	{
		UPromptSubsystem* PrompSubsytem = GetGameInstance()->GetSubsystem<UPromptSubsystem>();
		if (PrompSubsytem) 
		{
			PrompSubsytem->HideLoading();
		}
	}

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