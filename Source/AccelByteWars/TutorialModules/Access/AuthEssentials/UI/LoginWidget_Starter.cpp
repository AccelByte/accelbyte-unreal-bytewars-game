// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "LoginWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void ULoginWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	AuthSubsystem = GameInstance->GetSubsystem<UAuthEssentialsSubsystem_Starter>();
	ensure(AuthSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);

	SetLoginState(ELoginState::Default);
	OnRetryLoginDelegate.Clear();
}

void ULoginWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ThisClass::OnLoginWithDeviceIdButtonClicked);
	Btn_RetryLogin->OnClicked().AddUObject(this, &ThisClass::OnRetryLoginButtonClicked);
	Btn_QuitGame->OnClicked().AddUObject(this, &ThisClass::OnQuitGameButtonClicked);
}

void ULoginWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().Clear();
	Btn_RetryLogin->OnClicked().Clear();
	Btn_QuitGame->OnClicked().Clear();
}

void ULoginWidget_Starter::SetLoginState(const ELoginState NewState) const
{
	UWidget* WidgetToActivate;

	switch (NewState)
	{
	case ELoginState::Default:
		WidgetToActivate = Vb_LoginOptions;
		Btn_LoginWithDeviceId->SetUserFocus(GetOwningPlayer());
		InitializeFTUEDialogues(true);
		break;
	case ELoginState::LoggingIn:
		WidgetToActivate = Vb_LoginLoading;
		DeinitializeFTUEDialogues();
		HideFTUEDevHelpButton();
		break;
	case ELoginState::Failed:
		WidgetToActivate = Vb_LoginFailed;
		Btn_RetryLogin->SetUserFocus(GetOwningPlayer());
		DeinitializeFTUEDialogues();
		break;
	default:
		WidgetToActivate = Vb_LoginOptions;
		Btn_LoginWithDeviceId->SetUserFocus(GetOwningPlayer());
		InitializeFTUEDialogues(true);
		break;
	}

	Ws_LoginState->SetActiveWidget(WidgetToActivate);
}

void ULoginWidget_Starter::OnLoginWithDeviceIdButtonClicked()
{
	// TODO (TutorialModule): call login with device id.
	
	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Please integrate AcceByte Online Subsystem to access AccelByte Game Services."));
}

void ULoginWidget_Starter::OnRetryLoginButtonClicked()
{
	if (OnRetryLoginDelegate.IsBound())
	{
		OnRetryLoginDelegate.Broadcast();
	}
}

void ULoginWidget_Starter::OnQuitGameButtonClicked()
{
	ensure(PromptSubsystem);

	PromptSubsystem->ShowDialoguePopUp(
		LOCTEXT("Quit Game", "Quit Game"),
		LOCTEXT("Are you sure?", "Are you sure?"),
		EPopUpType::ConfirmationYesNo,
		FPopUpResultDelegate::CreateWeakLambda(this, [this](EPopUpResult Result)
		{
			if (Result == EPopUpResult::Confirmed)
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Type::Quit, false);
			}
		})
	);
}

void ULoginWidget_Starter::OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage)
{
	// TODO: Handle on login complete event.
	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("On login complete event is not yet implemented."));
}

void ULoginWidget_Starter::OpenMainMenu()
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Failed to open Main Menu. Base UI widget is null."));
		return;
	}

	// Proceed to show upgrade account if applicable, otherwise open Main Menu.
	UTutorialModuleDataAsset* DataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(RegisterUserInGameAssetId, this);
	BaseUIWidget->PushWidgetToStack(
		EBaseUIStackType::Menu,
		DataAsset && DataAsset->IsActiveAndDependenciesChecked() ?
		DataAsset->GetTutorialModuleUIClass() :
		MainMenuWidgetClass);
}

void ULoginWidget_Starter::SetButtonLoginVisibility(const ESlateVisibility InSlateVisibility) const
{
	Btn_LoginWithDeviceId->SetVisibility(InSlateVisibility);
}

#undef LOCTEXT_NAMESPACE