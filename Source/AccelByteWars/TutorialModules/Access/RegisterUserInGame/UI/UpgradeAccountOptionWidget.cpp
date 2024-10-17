// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UpgradeAccountOptionWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameLog.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

// @@@SNIPSTART UpgradeAccountOptionWidget.cpp-NativeConstruct
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-3", "11-16"], "highlightedLines": "{5-9}"}
void UUpgradeAccountOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RegisterUserInGameSubsystem = GameInstance->GetSubsystem<URegisterUserInGameSubsystem>();
	ensure(RegisterUserInGameSubsystem);

	SetVisibility(
		!RegisterUserInGameSubsystem->IsAllowUpgradeAccount() || 
		RegisterUserInGameSubsystem->IsCurrentUserIsFullAccount() ? 
		ESlateVisibility::Hidden : 
		ESlateVisibility::Visible);
}
// @@@SNIPEND

// @@@SNIPSTART UpgradeAccountOptionWidget.cpp-NativeOnActivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "10-14"]}
// @@@MULTISNIP PutItAllTogether {"highlightedLines": "{3-8}"}
void UUpgradeAccountOptionWidget::NativeOnActivated()
{
	if (!RegisterUserInGameSubsystem->IsAllowUpgradeAccount() || 
		RegisterUserInGameSubsystem->IsCurrentUserIsFullAccount())
	{
		SkipUpgradeAccount();
		return;
	}

	Super::NativeOnActivated();

	Btn_Upgrade->OnClicked().AddUObject(this, &ThisClass::ProceedToUpgradeAccount);
	Btn_Skip->OnClicked().AddUObject(this, &ThisClass::SkipUpgradeAccount);
}
// @@@SNIPEND

void UUpgradeAccountOptionWidget::NativeOnDeactivated()
{
	Btn_Upgrade->OnClicked().Clear();
	Btn_Skip->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UUpgradeAccountOptionWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_Upgrade;
}

// @@@SNIPSTART UpgradeAccountOptionWidget.cpp-ProceedToUpgradeAccount
void UUpgradeAccountOptionWidget::ProceedToUpgradeAccount()
{
	if (!GameInstance) 
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to proceed to upgrade account. Game instance is null."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to proceed to upgrade account. Base UI widget is null."));
		return;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, UpgradeAccountWidgetClass);
}
// @@@SNIPEND

// @@@SNIPSTART UpgradeAccountOptionWidget.cpp-SkipUpgradeAccount
void UUpgradeAccountOptionWidget::SkipUpgradeAccount()
{
	if (!GameInstance)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to skip upgrade account. Game instance is null."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_REGISTERUSERINGAME(Warning, TEXT("Failed to skip upgrade account. Base UI widget is null."));
		return;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, MainMenuWidgetClass);
}
// @@@SNIPEND
