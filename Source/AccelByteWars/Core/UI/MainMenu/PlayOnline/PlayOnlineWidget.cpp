// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "PlayOnlineWidget.h"

void UPlayOnlineWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

#pragma region "Is in demo mode or not"
	bool bDemoMode = false;
	bool bDemoModeSet = false;

	// launch param
	if (StaticCast<FString>(FCommandLine::Get()).Contains(TEXT("-DemoMode="), ESearchCase::IgnoreCase))
	{
		FString DemoMode;
		FParse::Value(FCommandLine::Get(), TEXT("-DemoMode="), DemoMode);
		if (DemoMode.Equals("true", ESearchCase::IgnoreCase))
		{
			bDemoMode = true;
			bDemoModeSet = true;
		}
		else if (DemoMode.Equals("false", ESearchCase::IgnoreCase))
		{
			bDemoMode = false;
			bDemoModeSet = true;
		}
	}

	// config DefaultEngine.ini
	if (!bDemoModeSet)
	{
		GConfig->GetBool(TEXT("ByteWars"), TEXT("bDemoMode"), bDemoMode, GEngineIni);
	}
#pragma endregion

	if (UUserWidget* Btn_CreateSession = FTutorialModuleGeneratedWidget::GetGeneratedWidgetById<UUserWidget>(
		TEXT("btn_create_session"), GeneratedWidgets))
	{
		Btn_CreateSession->SetVisibility(bDemoMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}
