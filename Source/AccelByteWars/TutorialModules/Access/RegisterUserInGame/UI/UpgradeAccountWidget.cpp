// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UpgradeAccountWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameModels.h"
#include "VerifyAccountWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

// @@@SNIPSTART UpgradeAccountWidget.cpp-NativeOnActivated
void UUpgradeAccountWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Upgrade->OnClicked().AddUObject(this, &ThisClass::UpgradeAccount);

	ToggleWarningText(false);
}
// @@@SNIPEND

void UUpgradeAccountWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();
	Btn_Upgrade->OnClicked().Clear();

	Edt_Username->SetText(FText::GetEmpty());
	Edt_Email->SetText(FText::GetEmpty());
	Edt_Password->SetText(FText::GetEmpty());
	Edt_RetypePassword->SetText(FText::GetEmpty());

	Super::NativeOnDeactivated();
}

UWidget* UUpgradeAccountWidget::NativeGetDesiredFocusTarget() const
{
	return Edt_Username;
}

// @@@SNIPSTART UpgradeAccountWidget.cpp-UpgradeAccount
void UUpgradeAccountWidget::UpgradeAccount()
{
	const FString Username = Edt_Username->GetText().ToString();
	const FString Email = Edt_Email->GetText().ToString();
	const FString Password = Edt_Password->GetText().ToString();
	const FString RetypePassword = Edt_RetypePassword->GetText().ToString();

	// Validate input.
	ToggleWarningText(false);
	if (Username.IsEmpty() || Email.IsEmpty() || Password.IsEmpty() || RetypePassword.IsEmpty()) 
	{
		ToggleWarningText(true, EMPTY_REQUIRED_FIELDS_ERROR);
		return;
	}
	else if (!IsValidUsername(Username)) 
	{
		ToggleWarningText(true, GetUpgradeAccountErrorMessage(EUpgradeAccountErrorTypes::UsernameInputViolation));
		return;
	}
	else if (!AccelByteWarsUtility::IsValidEmailAddress(Email))
	{
		ToggleWarningText(true, EMAIL_INPUT_VIOLATION_ERROR);
		return;
	}
	else if (!Password.Equals(RetypePassword, ESearchCase::Type::CaseSensitive))
	{
		ToggleWarningText(true, PASSWORD_NOT_MATCH_ERROR);
		return;
	}
	else if (!IsValidPassword(Password))
	{
		ToggleWarningText(true, GetUpgradeAccountErrorMessage(EUpgradeAccountErrorTypes::PasswordInputViolation));
		return;
	}

	ProceedToVerifyAccount();
}
// @@@SNIPEND

void UUpgradeAccountWidget::ToggleWarningText(bool bShow, const FText& Message)
{
	Tb_Warning->SetText(Message);
	Tb_Warning->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UUpgradeAccountWidget::ProceedToVerifyAccount()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to proceed to verify account. Game instance is null."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to proceed to verify account. Base UI widget is null."));
		return;
	}

	const FString Username = Edt_Username->GetText().ToString();
	const FString Email = Edt_Email->GetText().ToString();
	const FString Password = Edt_Password->GetText().ToString();

	if (UVerifyAccountWidget* VerifyAccountWidget = Cast<UVerifyAccountWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, VerifyAccountWidgetClass)))
	{
		FUpgradeAccountData UpgradeAccountData(Username, Email, Password);
		VerifyAccountWidget->SetAccountToVerify(UpgradeAccountData);
		UpgradeAccountData.Reset();
	}
}
