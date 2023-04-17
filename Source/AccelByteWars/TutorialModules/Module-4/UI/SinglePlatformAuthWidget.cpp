// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-4/UI/SinglePlatformAuthWidget.h"
#include "TutorialModules/Module-2/UI/LoginWidget.h"
#include "CommonButtonBase.h"

void USinglePlatformAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_LoginWithSinglePlatformAuth->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
	
	// Toggle the login button visibility if Steam OSS exists.
	SetVisibility(IOnlineSubsystem::GetByPlatform() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// Auto login with Steam.
	bool bShowLoginMenu = true;
	GConfig->GetBool(TEXT("AuthEssentialsLoginMenu"), TEXT("bEnabled"), bShowLoginMenu, GEngineIni);
	if (!bShowLoginMenu && GetVisibility() == ESlateVisibility::Visible)
	{
		OnLoginWithSinglePlatformAuthButtonClicked();
	}
}

void USinglePlatformAuthWidget::OnLoginWithSinglePlatformAuthButtonClicked()
{
	ULoginWidget::OnLoginWithSinglePlatformAuthDelegate.ExecuteIfBound();
}