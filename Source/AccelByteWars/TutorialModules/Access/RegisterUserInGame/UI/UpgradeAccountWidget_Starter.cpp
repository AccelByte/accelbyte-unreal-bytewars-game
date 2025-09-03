// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UpgradeAccountWidget_Starter.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameModels.h"
#include "VerifyAccountWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UUpgradeAccountWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RegisterUserInGameSubsystem = GameInstance->GetSubsystem<URegisterUserInGameSubsystem_Starter>();
	ensure(RegisterUserInGameSubsystem);

	bResetInputOnDeactivated = true;
	ToggleWarningText(false);

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	// TODO: Bind your widget functions here.
}

void UUpgradeAccountWidget_Starter::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();
	Btn_Upgrade->OnClicked().Clear();

	if (bResetInputOnDeactivated)
	{
		Edt_Username->SetText(FText::GetEmpty());
		Edt_DisplayName->SetText(FText::GetEmpty());
		Edt_Email->SetText(FText::GetEmpty());
		Edt_Password->SetText(FText::GetEmpty());
		Edt_RetypePassword->SetText(FText::GetEmpty());
	}

	Super::NativeOnDeactivated();
}

UWidget* UUpgradeAccountWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Edt_Username;
}

void UUpgradeAccountWidget_Starter::ToggleWarningText(bool bShow, const FText& Message)
{
	Tb_Warning->SetText(Message);
	Tb_Warning->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UUpgradeAccountWidget_Starter::ProceedToVerifyAccount()
{
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to proceed to verify account. Base UI widget is null."));
		return;
	}

	const FString Username = Edt_Username->GetText().ToString();
	const FString DisplayName = Edt_DisplayName->GetText().ToString();
	const FString Email = Edt_Email->GetText().ToString();
	const FString Password = Edt_Password->GetText().ToString();
	bResetInputOnDeactivated = false;

	if (UVerifyAccountWidget_Starter* VerifyAccountWidget = Cast<UVerifyAccountWidget_Starter>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, VerifyAccountWidgetClass)))
	{
		FUpgradeAccountData UpgradeAccountData(Username, DisplayName, Email, Password);
		VerifyAccountWidget->SetAccountToVerify(UpgradeAccountData);
		UpgradeAccountData.Reset();
	}
}

#pragma region Module Register User In-Game Function Definitions
// TODO: Add your function definitions here.
#pragma endregion