// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-7/UI/SinglePlatformAuthWidget_Starter.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem.h"
#include "TutorialModules/Module-2/UI/LoginWidget.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem_Starter.h"
#include "TutorialModules/Module-2/UI/LoginWidget_Starter.h"

void USinglePlatformAuthWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

	// TODO: Put the auto login with single auth platform implementation here.
}

void USinglePlatformAuthWidget_Starter::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().Clear();
}

void USinglePlatformAuthWidget_Starter::OnLoginWithSinglePlatformAuthButtonClicked()
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
		// TODO: Trigger login with single auth platform here.
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Login with single auth platform is not yet implemented"));
	}
}