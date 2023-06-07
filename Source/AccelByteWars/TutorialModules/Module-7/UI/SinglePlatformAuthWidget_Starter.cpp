// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-7/UI/SinglePlatformAuthWidget_Starter.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem_Starter.h"
#include "TutorialModules/Module-2/UI/LoginWidget_Starter.h"
#include "CommonButtonBase.h"

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
	// TODO: Trigger login with single auth platform here.
}