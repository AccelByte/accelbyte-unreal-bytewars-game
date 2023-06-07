// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-7/UI/SinglePlatformAuthWidget.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem.h"
#include "TutorialModules/Module-2/UI/LoginWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void USinglePlatformAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

	// Toggle the login button visibility if the default native platform exists.
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
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	UAuthEssentialsSubsystem* AuthSubsystem = GameInstance->GetSubsystem<UAuthEssentialsSubsystem>();
	ensure(AuthSubsystem);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	// Grab the login widget reference to show login state UI.
	ULoginWidget* LoginWidget = Cast<ULoginWidget>(BaseUIWidget->Stacks[EBaseUIStackType::Menu]->GetActiveWidget());
	ensure(LoginWidget);

	LoginWidget->SetLoginState(ELoginState::LoggingIn);
	LoginWidget->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);

	// Login with single platform auth is considered as login with default native platform.
	// Thus, it doesn't need username, token, nor the login method.
	AuthSubsystem->SetAuthCredentials(EAccelByteLoginType::None, TEXT(""), TEXT(""));
	AuthSubsystem->Login(PC, FAuthOnLoginCompleteDelegate::CreateUObject(LoginWidget, &ULoginWidget::OnLoginComplete));
}