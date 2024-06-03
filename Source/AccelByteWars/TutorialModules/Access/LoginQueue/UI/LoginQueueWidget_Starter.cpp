// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "LoginQueueWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Access/AuthEssentials/UI/LoginWidget_Starter.h"
#include "Access/LoginQueue/LoginQueueLog.h"
#include "Access/LoginQueue/LoginQueueSubsystem_Starter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void ULoginQueueWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	LoginQueueSubsystem = GetGameInstance()->GetSubsystem<ULoginQueueSubsystem_Starter>();
	ensure(LoginQueueSubsystem);

	W_Parent = GetFirstOccurenceOuter<ULoginWidget_Starter>();
	if (!ensure(W_Parent))
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("Activate Auth Essentials's starter mode or deactivate this module's starter mode to make this widget work properly."))
		return;
	}

#pragma region "Tutorial"
	// place your code here
#pragma endregion
}

void ULoginQueueWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "Tutorial"
	// place your code here
#pragma endregion
}

#pragma region "Tutorial"
// place your code here
#pragma endregion
