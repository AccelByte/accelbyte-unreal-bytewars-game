// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SinglePlatformAuthWidget_Starter.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Access/AuthEssentials/AuthEssentialsSubsystem_Starter.h"
#include "Access/AuthEssentials/UI/LoginWidget_Starter.h"

void USinglePlatformAuthWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	const FPrimaryAssetId AuthEssentialsAssetId = FPrimaryAssetId("TutorialModule:AUTHESSENTIALS");
	AuthEssentialsModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(AuthEssentialsAssetId, this);
	ensure(AuthEssentialsModule);
	
	ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	ensure(ParentWidget);

	LoginWidget = Cast<ULoginWidget_Starter>(ParentWidget);
	ensure(LoginWidget);
}

void USinglePlatformAuthWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!AuthEssentialsModule->IsStarterModeActive())
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("SinglePlatformAuth is dependent to AuthEssentials, please enable starter mode for the AuthEssentials module."))
		Btn_LoginWithSinglePlatformAuth->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const bool bIsNativePlatformValid = IOnlineSubsystem::GetByPlatform() != nullptr;
	if (!bIsNativePlatformValid)
	{
		Btn_LoginWithSinglePlatformAuth->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	// TODO: Implement single platform auth widget interaction and auto login here.
	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Single platform auth login is not yet implemented"));
}

void USinglePlatformAuthWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_LoginWithSinglePlatformAuth->OnClicked().Clear();
}

void USinglePlatformAuthWidget_Starter::OnLoginWithSinglePlatformAuthButtonClicked()
{
	// TODO: Trigger login with single auth platform here.
	UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Login with single platform auth is not yet implemented"));
}

bool USinglePlatformAuthWidget_Starter::ShouldAutoLogin()
{
	bool bAutoLogin = false;
	const FString AutoLoginKey = "bAutoLogin";
	GConfig->GetBool(AUTH_ESSENTIALS_SECTION, *AutoLoginKey, bAutoLogin, GEngineIni);
	FParse::Bool(FCommandLine::Get(), *FString::Printf(TEXT("-%s="), *AutoLoginKey), bAutoLogin);
	return bAutoLogin;
}

bool USinglePlatformAuthWidget_Starter::ShouldDisplayDeviceIdLogin()
{
	bool bAllowDeviceIdLogin = false;
	const FString AllowDeviceIdLoginKey = "bAllowDeviceIdLogin";
	GConfig->GetBool(SINGLE_PLATFORM_AUTH_SECTION, *AllowDeviceIdLoginKey, bAllowDeviceIdLogin, GEngineIni);
	FParse::Bool(FCommandLine::Get(), *FString::Printf(TEXT("-%s="), *AllowDeviceIdLoginKey), bAllowDeviceIdLogin);
	return bAllowDeviceIdLogin;
}

FString USinglePlatformAuthWidget_Starter::GetDefaultNativePlatform()
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