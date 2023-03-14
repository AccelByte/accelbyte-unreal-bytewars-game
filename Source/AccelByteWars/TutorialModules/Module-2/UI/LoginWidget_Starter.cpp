// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-2/UI/LoginWidget_Starter.h"
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

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void ULoginWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_LoginWithDeviceId->OnClicked().AddUObject(this, &ULoginWidget_Starter::OnLoginWithDeviceIdButtonClicked);
	Btn_RetryLogin->OnClicked().AddUObject(this, &ULoginWidget_Starter::OnRetryLoginButtonClicked);
	Btn_QuitGame->OnClicked().AddUObject(this, &ULoginWidget_Starter::OnQuitGameButtonClicked);

	SetLoginState(ELoginState::Default);
}

void ULoginWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithDeviceId->OnClicked().RemoveAll(this);
	Btn_RetryLogin->OnClicked().RemoveAll(this);
	Btn_QuitGame->OnClicked().RemoveAll(this);
}

void ULoginWidget_Starter::SetLoginState(const ELoginState NewState)
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

void ULoginWidget_Starter::OnLoginWithDeviceIdButtonClicked()
{
	// TODO (TutorialModule): call login with device id.
	
	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Please integrate AcceByte Online Subsystem to access AccelByte Game Services."));
}

void ULoginWidget_Starter::OnRetryLoginButtonClicked()
{
	// TODO (TutorialModule): call login with latest login method used

	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Please integrate AcceByte Online Subsystem to access AccelByte Game Services."));
}

void ULoginWidget_Starter::OnQuitGameButtonClicked()
{
	QuitGame();
}

void ULoginWidget_Starter::QuitGame()
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