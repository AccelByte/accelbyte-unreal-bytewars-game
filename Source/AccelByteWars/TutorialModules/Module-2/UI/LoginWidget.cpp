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

	SetLoginState(ELoginState::Default);
	OnRetryLoginDelegate.Clear();
}

void ULoginWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ThisClass::OnLoginWithDeviceIdButtonClicked);
	Btn_RetryLogin->OnClicked().AddUObject(this, &ThisClass::OnRetryLoginButtonClicked);
	Btn_QuitGame->OnClicked().AddUObject(this, &ThisClass::OnQuitGameButtonClicked);

	FFTUEDialogueModel* FTUEDialogue = FFTUEDialogueModel::GetMetadataById("test_ftue", FTUEDialogues);
	if (FTUEDialogue) 
	{
		FTUEDialogue->Button1.ButtonActionDelegate.RemoveAll(this);
		FTUEDialogue->Button1.ButtonActionDelegate.AddWeakLambda(this, []()
		{
			UE_LOG(LogTemp, Warning, TEXT("Hello World Nani Kore Wow!"));
		});
	}

	AutoLoginCmd();
}

void ULoginWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().Clear();
	Btn_RetryLogin->OnClicked().Clear();
	Btn_QuitGame->OnClicked().Clear();
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

#undef LOCTEXT_NAMESPACE