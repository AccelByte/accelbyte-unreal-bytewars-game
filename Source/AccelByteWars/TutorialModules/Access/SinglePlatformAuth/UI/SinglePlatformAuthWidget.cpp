// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SinglePlatformAuthWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"

void USinglePlatformAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const FPrimaryAssetId AuthEssentialsAssetId = FPrimaryAssetId("TutorialModule:AUTHESSENTIALS");
	AuthEssentialsModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(AuthEssentialsAssetId, this);
	ensure(AuthEssentialsModule);

	ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	ensure(ParentWidget);

	LoginWidget = Cast<ULoginWidget>(ParentWidget);
	ensure(LoginWidget);
}

void USinglePlatformAuthWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (AuthEssentialsModule->IsStarterModeActive())
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("AuthEssentials is in starter mode and SinglePlatformAuth is dependent to it, please also enable the starter mode for the SinglePlatformAuth module."))
		Btn_LoginWithSinglePlatformAuth->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const bool bIsNativePlatformValid = IOnlineSubsystem::GetByPlatform() != nullptr;
	if (!bIsNativePlatformValid)
	{
		Btn_LoginWithSinglePlatformAuth->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ShouldAutoLogin())
	{
		OnLoginWithSinglePlatformAuthButtonClicked();
		return;
	}

	const FString LoginButtonText = TEXT_LOGIN_WITH.ToString().Replace(TEXT("%PLATFORM%"), *GetDefaultNativePlatform());
	Btn_LoginWithSinglePlatformAuth->SetButtonText(FText::FromString(LoginButtonText));
	Btn_LoginWithSinglePlatformAuth->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
	LoginWidget->SetButtonLoginVisibility(ShouldDisplayDeviceIdLogin() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void USinglePlatformAuthWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithSinglePlatformAuth->OnClicked().Clear();
}

void USinglePlatformAuthWidget::OnLoginWithSinglePlatformAuthButtonClicked()
{
	UAuthEssentialsSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthEssentialsSubsystem>();
	ensure(AuthSubsystem);

	// Set the login widget to logging in state and bind the retry login delegate.
	LoginWidget->SetLoginState(ELoginState::LoggingIn);
	LoginWidget->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

	// Login with single platform auth is considered as login with default native platform.
	// Thus, it doesn't need username, token, nor the login method.
	AuthSubsystem->SetAuthCredentials(EAccelByteLoginType::None, TEXT(""), TEXT(""));
	AuthSubsystem->Login(GetOwningPlayer(), FAuthOnLoginCompleteDelegate::CreateUObject(LoginWidget, &ULoginWidget::OnLoginComplete));
}

bool USinglePlatformAuthWidget::ShouldAutoLogin()
{
	bool bAutoLogin = false;
	const FString AutoLoginKey = "bAutoLogin";
	GConfig->GetBool(AUTH_ESSENTIALS_SECTION, *AutoLoginKey, bAutoLogin, GEngineIni);
	FParse::Bool(FCommandLine::Get(), *FString::Printf(TEXT("-%s="), *AutoLoginKey), bAutoLogin);
	return bAutoLogin;
}

bool USinglePlatformAuthWidget::ShouldDisplayDeviceIdLogin()
{
	bool bAllowDeviceIdLogin = false;
	const FString AllowDeviceIdLoginKey = "bAllowDeviceIdLogin";
	GConfig->GetBool(SINGLE_PLATFORM_AUTH_SECTION, *AllowDeviceIdLoginKey, bAllowDeviceIdLogin, GEngineIni);
	FParse::Bool(FCommandLine::Get(), *FString::Printf(TEXT("-%s="), *AllowDeviceIdLoginKey), bAllowDeviceIdLogin);
	return bAllowDeviceIdLogin;
}

FString USinglePlatformAuthWidget::GetDefaultNativePlatform()
{
	FString DefaultNativePlatform;
	const FString OnlineSubsystemSection = "OnlineSubsystem";
	const FString NativePlatformKey = "NativePlatformService";
	GConfig->GetString(*OnlineSubsystemSection, *NativePlatformKey, DefaultNativePlatform, GEngineIni);
	return DefaultNativePlatform;
}

#undef TEXT_LOGIN_WITH
#undef AUTH_ESSENTIALS_SECTION
#undef SINGLE_PLATFORM_AUTH_SECTION