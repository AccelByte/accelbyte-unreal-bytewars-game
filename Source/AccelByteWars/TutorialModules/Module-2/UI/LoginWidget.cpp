// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-2/UI/LoginWidget.h"
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
}

void ULoginWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ULoginWidget::OnLoginWithDeviceIdButtonClicked);
	Btn_RetryLogin->OnClicked().AddUObject(this, &ULoginWidget::OnRetryLoginButtonClicked);
	Btn_QuitGame->OnClicked().AddUObject(this, &ULoginWidget::OnQuitGameButtonClicked);

	SetLoginState(ELoginState::Default);

	AutoLoginCmd();
}

void ULoginWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().RemoveAll(this);
	Btn_RetryLogin->OnClicked().RemoveAll(this);
	Btn_QuitGame->OnClicked().RemoveAll(this);
}

void ULoginWidget::SetLoginState(const ELoginState NewState)
{
	Ws_LoginState->SetActiveWidgetIndex((int)NewState);

	switch (NewState)
	{
		case ELoginState::Default:
			Btn_LoginWithDeviceId->SetUserFocus(GetOwningPlayer());
			break;
		case ELoginState::Failed:
			Btn_RetryLogin->SetUserFocus(GetOwningPlayer());
			break;
		default:
			Btn_LoginWithDeviceId->SetUserFocus(GetOwningPlayer());
			break;
	}
}

void ULoginWidget::OnLoginWithDeviceIdButtonClicked()
{
	Login(EAccelByteLoginType::DeviceId);
}

void ULoginWidget::OnRetryLoginButtonClicked()
{
	Login(LastLoginMethod);
}

void ULoginWidget::OnQuitGameButtonClicked()
{
	QuitGame();
}

void ULoginWidget::Login(EAccelByteLoginType LoginMethod)
{
	SetLoginState(ELoginState::LoggingIn);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);
	
	// Cached last login method for the UI login retry functionality later. 
	LastLoginMethod = LoginMethod;

	ensure(AuthSubsystem);
	AuthSubsystem->Login(LoginMethod, PC, FAuthOnLoginCompleteDelegate::CreateUObject(this, &ULoginWidget::OnLoginComplete));
}

void ULoginWidget::OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage)
{
	if (bWasSuccessful) 
	{
		// When login success, open Main Menu widget.
		UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->BaseUIWidget);
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
	FString CmdArgs = FCommandLine::Get();
	if (!CmdArgs.Contains(TEXT("-AUTH_TYPE=ACCELBYTE")))
	{
		return;
	}
	FString Username, Password;
	FParse::Value(FCommandLine::Get(), TEXT("-AUTH_LOGIN="), Username);
	FParse::Value(FCommandLine::Get(), TEXT("-AUTH_PASSWORD="), Password);
	if (!Username.IsEmpty() && !Password.IsEmpty())
	{
		AuthSubsystem->SetAuthCredentials(Username, Password);
		Login(EAccelByteLoginType::AccelByte);
	}
	else
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot auto login with AccelByte account. Username and password cannot be found from command line."));
	}
}

void ULoginWidget::QuitGame()
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

#undef LOCTEXT_NAMESPACE