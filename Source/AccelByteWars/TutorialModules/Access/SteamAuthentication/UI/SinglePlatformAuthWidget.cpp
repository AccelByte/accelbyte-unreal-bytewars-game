// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Access/SteamAuthentication/UI/SinglePlatformAuthWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"
#include "Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"
#include "Access/AuthEssentials/AuthEssentialsSubsystem_Starter.h"
#include "Access/AuthEssentials/UI/LoginWidget_Starter.h"

void USinglePlatformAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().Clear();
	Btn_LoginWithSinglePlatformAuth->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

	// Toggle the login button visibility if the default native platform exists.
	Btn_LoginWithSinglePlatformAuth->SetVisibility(IOnlineSubsystem::GetByPlatform() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SetVisibility(IOnlineSubsystem::GetByPlatform() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Auto login with default native platform.
	bool bAutoLogin = true;
	GConfig->GetBool(TEXT("/ByteWars/TutorialModule.AuthEssentials"), TEXT("bAutoLogin"), bAutoLogin, GEngineIni);
	if (bAutoLogin && GetVisibility() == ESlateVisibility::Visible)
	{
		OnLoginWithSinglePlatformAuthButtonClicked();
	}
}

void USinglePlatformAuthWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().Clear();
}

void USinglePlatformAuthWidget::OnLoginWithSinglePlatformAuthButtonClicked()
{
	UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (!ParentWidget)
	{
		return;
	}

	UTutorialModuleDataAsset* AuthEssentialsModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId("TutorialModule:AUTHESSENTIALS"), this);
	if (!AuthEssentialsModule) 
	{
		return;
	}

	// Use Auth Essentials's default files for login if starter mode is not active.
	if (!AuthEssentialsModule->IsStarterModeActive()) 
	{
		UAuthEssentialsSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthEssentialsSubsystem>();
		ensure(AuthSubsystem);

		// Grab the login widget reference to show login state UI.
		ULoginWidget* LoginWidget = Cast<ULoginWidget>(ParentWidget);
		ensure(LoginWidget);

		LoginWidget->SetLoginState(ELoginState::LoggingIn);
		LoginWidget->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

		// Login with single platform auth is considered as login with default native platform.
		// Thus, it doesn't need username, token, nor the login method.
		AuthSubsystem->SetAuthCredentials(EAccelByteLoginType::None, TEXT(""), TEXT(""));
		AuthSubsystem->Login(GetOwningPlayer(), FAuthOnLoginCompleteDelegate::CreateUObject(LoginWidget, &ULoginWidget::OnLoginComplete));
	}
	// Use Auth Essentials's starter files for login if starter mode is active.
	else 
	{
		UAuthEssentialsSubsystem_Starter* AuthSubsystem_Starter = GetGameInstance()->GetSubsystem<UAuthEssentialsSubsystem_Starter>();
		ensure(AuthSubsystem_Starter);

		// Grab the login widget reference to show login state UI.
		ULoginWidget_Starter* LoginWidget_Starter = Cast<ULoginWidget_Starter>(ParentWidget);
		ensure(LoginWidget_Starter);

		LoginWidget_Starter->SetLoginState(ELoginState::LoggingIn);
		LoginWidget_Starter->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

		// Login with single platform auth is considered as login with default native platform.
		// Thus, it doesn't need username, token, nor the login method.
		AuthSubsystem_Starter->SetAuthCredentials(EAccelByteLoginType::None, TEXT(""), TEXT(""));
		AuthSubsystem_Starter->Login(GetOwningPlayer(), FAuthOnLoginCompleteDelegate_Starter::CreateUObject(LoginWidget_Starter, &ULoginWidget_Starter::OnLoginComplete));
	}
}