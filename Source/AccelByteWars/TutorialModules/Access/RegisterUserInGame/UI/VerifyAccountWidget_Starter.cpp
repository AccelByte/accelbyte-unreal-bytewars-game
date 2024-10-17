// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "VerifyAccountWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UVerifyAccountWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RegisterUserInGameSubsystem = GameInstance->GetSubsystem<URegisterUserInGameSubsystem_Starter>();
	ensure(RegisterUserInGameSubsystem);

	Ws_VerifyAccount->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UVerifyAccountWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	ToggleWarningText(false);

	// TODO: Add your function call here.
}

void UVerifyAccountWidget_Starter::NativeOnDeactivated()
{
	// TODO: Add your function call here.

	UpgradeAccountData.Reset();
	Edt_VerificationCode->SetText(FText::GetEmpty());

	Btn_Back->OnClicked().Clear();
	Btn_Resend->OnClicked().Clear();
	Btn_Verify->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

void UVerifyAccountWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// TODO: Add your function call here.
}

UWidget* UVerifyAccountWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Edt_VerificationCode;
}

void UVerifyAccountWidget_Starter::SetAccountToVerify(const FUpgradeAccountData& InUpgradeAccountData)
{
	UpgradeAccountData = InUpgradeAccountData;
}

void UVerifyAccountWidget_Starter::ToggleWarningText(bool bShow, const FText& Message)
{
	Tb_Warning->SetText(Message);
	Tb_Warning->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UVerifyAccountWidget_Starter::OpenToMainMenu()
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

#pragma region Module Register User In-Game Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
