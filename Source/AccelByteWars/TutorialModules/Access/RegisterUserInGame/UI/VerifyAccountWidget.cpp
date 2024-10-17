// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "VerifyAccountWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UVerifyAccountWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RegisterUserInGameSubsystem = GameInstance->GetSubsystem<URegisterUserInGameSubsystem>();
	ensure(RegisterUserInGameSubsystem);

	Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
}

// @@@SNIPSTART VerifyAccountWidget.cpp-NativeOnActivated
void UVerifyAccountWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Resend->OnClicked().AddUObject(this, &ThisClass::SendVerificationCode, true);
	Btn_Verify->OnClicked().AddUObject(this, &ThisClass::UpgradeAndVerifyAccount);

	ToggleWarningText(false);
	SendVerificationCode(false);
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-NativeOnDeactivated
void UVerifyAccountWidget::NativeOnDeactivated()
{
	ResetRequestVerificationCodeCooldown();

	UpgradeAccountData.Reset();
	Edt_VerificationCode->SetText(FText::GetEmpty());

	Btn_Back->OnClicked().Clear();
	Btn_Resend->OnClicked().Clear();
	Btn_Verify->OnClicked().Clear();

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-NativeTick
void UVerifyAccountWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateRequestVerificationCodeCooldown(InDeltaTime);
}
// @@@SNIPEND

UWidget* UVerifyAccountWidget::NativeGetDesiredFocusTarget() const
{
	return Edt_VerificationCode;
}

void UVerifyAccountWidget::SetAccountToVerify(const FUpgradeAccountData& InUpgradeAccountData)
{
	UpgradeAccountData = InUpgradeAccountData;
}

// @@@SNIPSTART VerifyAccountWidget.cpp-SendVerificationCode
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "25"]}
void UVerifyAccountWidget::SendVerificationCode(const bool bForceResend)
{
	ToggleWarningText(false);
	Ws_VerifyAccount->LoadingMessage = SEND_VERIFICATION_CODE_MESSAGE;
	Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	Btn_Resend->SetIsEnabled(false);
	RegisterUserInGameSubsystem->SendUpgradeAccountVerificationCode(
		UpgradeAccountData.GetEmail(),
		bForceResend,
		FOnSendUpgradeAccountVerificationCodeComplete::CreateWeakLambda(this, [this](bool bWasSuccesful, const FString& ErrorMessage)
		{
			Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

			if (!bWasSuccesful) 
			{
				Btn_Resend->SetIsEnabled(true);
				ToggleWarningText(true, FText::FromString(ErrorMessage));
				return;
			}

			StartRequestVerificationCodeCooldown();
		})
	);
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-UpgradeAndVerifyAccount
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "35"]}
void UVerifyAccountWidget::UpgradeAndVerifyAccount()
{
	const FString VerificationCode = Edt_VerificationCode->GetText().ToString();
	if (VerificationCode.IsEmpty()) 
	{
		ToggleWarningText(true, EMPTY_VERIFICATION_CODE_ERROR);
		return;
	}

	ToggleWarningText(false);
	Ws_VerifyAccount->LoadingMessage = UPGRADE_ACCOUNT_MESSAGE;
	Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	RegisterUserInGameSubsystem->UpgradeAndVerifyAccount(
		AccelByteWarsUtility::GetLocalUserNum(GetOwningPlayer()),
		AccelByteWarsUtility::GetUserId(GetOwningPlayer()),
		UpgradeAccountData.GetUsername(),
		UpgradeAccountData.GetEmail(),
		UpgradeAccountData.GetPassword(),
		VerificationCode,
		FOnUpgradeAndVerifyAccountComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& ErrorMessage, const FAccountUserData& NewFullAccount)
		{
			Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

			if (!bWasSuccessful)
			{
				ToggleWarningText(true, FText::FromString(ErrorMessage));
				return;
			}

			UpgradeAccountData.Reset();
			OpenToMainMenu();
		})
	);
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-StartRequestVerificationCodeCooldown
void UVerifyAccountWidget::StartRequestVerificationCodeCooldown()
{
	Btn_Resend->SetIsEnabled(false);
	RequestVerificationCodeTimer = (float)RequestVerificationCodeCooldown;
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-UpdateRequestVerificationCodeCooldown
void UVerifyAccountWidget::UpdateRequestVerificationCodeCooldown(float DeltaTime)
{
	if (RequestVerificationCodeTimer <= 0)
	{
		return;
	}

	RequestVerificationCodeTimer -= DeltaTime;
	Btn_Resend->SetButtonText(FText::FromString(
		FString::Printf(TEXT("%s (%d s)"), 
		*RESEND_VERIFICATION_CODE_MESSAGE.ToString(), 
		(int32)RequestVerificationCodeTimer)));

	if (RequestVerificationCodeTimer <= 0) 
	{
		ResetRequestVerificationCodeCooldown();
	}
}
// @@@SNIPEND

// @@@SNIPSTART VerifyAccountWidget.cpp-ResetRequestVerificationCodeCooldown
void UVerifyAccountWidget::ResetRequestVerificationCodeCooldown()
{
	if (!GetWorld() || GetWorld()->bIsTearingDown) 
	{
		return;
	}

	RequestVerificationCodeTimer = 0.0f;
	Btn_Resend->SetIsEnabled(true);
	Btn_Resend->SetButtonText(RESEND_VERIFICATION_CODE_MESSAGE);
}
// @@@SNIPEND

void UVerifyAccountWidget::ToggleWarningText(bool bShow, const FText& Message)
{
	Tb_Warning->SetText(Message);
	Tb_Warning->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UVerifyAccountWidget::OpenToMainMenu()
{
	if (!GameInstance)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to open to Main Menu. Game instance is null."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to open to Main Menu. Base UI widget is null."));
		return;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, MainMenuWidgetClass);
}