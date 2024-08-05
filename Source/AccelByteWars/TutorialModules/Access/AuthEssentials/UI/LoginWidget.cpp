// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "LoginWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	AuthSubsystem = GameInstance->GetSubsystem<UAuthEssentialsSubsystem>();
	ensure(AuthSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);

	SetLoginState(ELoginState::Default);
	OnRetryLoginDelegate.Clear();
}

void ULoginWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ThisClass::OnLoginWithDeviceIdButtonClicked);
	Btn_RetryLogin->OnClicked().AddUObject(this, &ThisClass::OnRetryLoginButtonClicked);
	Btn_QuitGame->OnClicked().AddUObject(this, &ThisClass::OnQuitGameButtonClicked);

	AutoLoginCmd();
}

void ULoginWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().Clear();
	Btn_RetryLogin->OnClicked().Clear();
	Btn_QuitGame->OnClicked().Clear();
}

void ULoginWidget::SetLoginState(const ELoginState NewState) const
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

void ULoginWidget::OnLoginWithDeviceIdButtonClicked()
{
	SetLoginState(ELoginState::LoggingIn);
	OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithDeviceIdButtonClicked);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);

	ensure(AuthSubsystem);
	AuthSubsystem->SetAuthCredentials(EAccelByteLoginType::DeviceId, TEXT(""), TEXT(""));
	AuthSubsystem->Login(PC, FAuthOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
}

void ULoginWidget::OnRetryLoginButtonClicked()
{
	if (OnRetryLoginDelegate.IsBound()) 
	{
		OnRetryLoginDelegate.Broadcast();
	}
}

void ULoginWidget::OnQuitGameButtonClicked()
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

void ULoginWidget::OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage)
{
	if (bWasSuccessful) 
	{
		// Broadcast on-login success event.
		if (UAuthEssentialsModels::OnLoginSuccessDelegate.IsBound())
		{
			UAuthEssentialsModels::OnLoginSuccessDelegate.Broadcast(GetOwningPlayer());
		}

		// When login success, open Main Menu widget.
		UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
		ensure(BaseUIWidget);
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, MainMenuWidgetClass);
	}
	else 
	{
		// When login failed, show error message.
		Tb_FailedMessage->SetText(FText::FromString(ErrorMessage));
		SetLoginState(ELoginState::Failed);
	}
}

void ULoginWidget::AutoLoginCmd()
{
	const FString AccelByteLoginTypeCmd = FString::Printf(TEXT("-%s=%s"), AUTH_TYPE_PARAM, AUTH_TYPE_ACCELBYTE_PARAM);
	const FString AccelByteLoginUsernameCmd = FString::Printf(TEXT("-%s="), AUTH_LOGIN_PARAM);
	const FString AccelByteLoginPasswordCmd = FString::Printf(TEXT("-%s="), AUTH_PASSWORD_PARAM);

	const FString CmdArgs = FCommandLine::Get();
	if (!CmdArgs.Contains(AccelByteLoginTypeCmd, ESearchCase::IgnoreCase))
	{
		return;
	}
	FString Username, Password;
	FParse::Value(FCommandLine::Get(), *AccelByteLoginUsernameCmd, Username);
	FParse::Value(FCommandLine::Get(), *AccelByteLoginPasswordCmd, Password);
	if (!Username.IsEmpty() && !Password.IsEmpty())
	{
		SetLoginState(ELoginState::LoggingIn);
		OnRetryLoginDelegate.AddUObject(this, &ThisClass::AutoLoginCmd);

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		ensure(PC);

		AuthSubsystem->SetAuthCredentials(EAccelByteLoginType::AccelByte, Username, Password);
		AuthSubsystem->Login(PC, FAuthOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
	}
	else
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot auto login with AccelByte account. Username and password cannot be found from command line."));
	}
}

void ULoginWidget::SetButtonLoginVisibility(const ESlateVisibility InSlateVisibility) const
{
	Btn_LoginWithDeviceId->SetVisibility(InSlateVisibility);
}

#undef LOCTEXT_NAMESPACE