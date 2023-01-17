// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Access/AuthEssentials/UI/LoginWidget.h"

void ULoginWidget::Login(EAccelByteLoginType LoginMethod, const FAuthOnLoginComplete& OnLoginComplete)
{
	UAuthEssentialsSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthEssentialsSubsystem>();
	if (!ensure(AuthSubsystem)) 
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot login. Auth Essentials Subsystem is not valid."));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!ensure(PC)) 
	{
		UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot login. Player Controller is not valid."));
		return;
	}
	
	LastLoginMethod = LoginMethod;

	AuthSubsystem->Login(LoginMethod, PC, OnLoginComplete);
}