// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Access/AuthEssentials/UI/LoginWidget.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"
#include "ByteWarsCore/UI/Components/Prompt/PromptSubsystem.h"
#include "ByteWarsCore/UI/AccelByteWarsBaseUI.h"

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

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ULoginWidget::Login, EAccelByteLoginType::DeviceId);
	Btn_RetryLogin->OnClicked().AddWeakLambda(this, [this]() { Login(LastLoginMethod); });
	Btn_QuitGame->OnClicked().AddUObject(this, &ULoginWidget::QuitGame);

	SetLoginState(ELoginState::Default);
}

void ULoginWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().RemoveAll(this);
	Btn_RetryLogin->OnClicked().RemoveAll(this);
	Btn_QuitGame->OnClicked().RemoveAll(this);
}

void ULoginWidget::Login(EAccelByteLoginType LoginMethod)
{
	SetLoginState(ELoginState::LoggingIn);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);
	
	LastLoginMethod = LoginMethod;

	ensure(AuthSubsystem);
	AuthSubsystem->Login(LoginMethod, PC, FAuthOnLoginCompleteDelegate::CreateUObject(this, &ULoginWidget::OnLoginComplete));
}

void ULoginWidget::SetLoginState(const ELoginState NewState)
{
	Ws_LoginState->SetActiveWidgetIndex(NewState);

	switch (NewState)
	{
		case ELoginState::Default:
			if (B_DesktopLogin->IsVisible()) 
			{
				Btn_LoginWithDeviceId->SetUserFocus(GetOwningPlayer());
			}
			else
			{
				Btn_LoginWithConsole->SetUserFocus(GetOwningPlayer());
			}
			break;
		case ELoginState::Failed:
			Btn_RetryLogin->SetUserFocus(GetOwningPlayer());
			break;
	}
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