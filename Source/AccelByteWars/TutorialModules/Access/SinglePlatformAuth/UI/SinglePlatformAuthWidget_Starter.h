// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "SinglePlatformAuthWidget_Starter.generated.h"

class ULoginWidget_Starter;
class UAccelByteWarsButtonBase;
class UAuthEssentialsSubsystem_Starter;

#define TEXT_LOGIN_WITH NSLOCTEXT("AccelByteWars", "login_with", "Login with %PLATFORM%")
#define AUTH_ESSENTIALS_SECTION TEXT("/ByteWars/TutorialModule.AuthEssentials")
#define SINGLE_PLATFORM_AUTH_SECTION TEXT("/ByteWars/TutorialModule.SinglePlatformAuth")

UCLASS(Abstract)
class ACCELBYTEWARS_API USinglePlatformAuthWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	void OnLoginWithSinglePlatformAuthButtonClicked();
	
	static bool ShouldAutoLogin();
	static bool ShouldDisplayDeviceIdLogin();
	static FString GetDefaultNativePlatform();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_LoginWithSinglePlatformAuth;

	UPROPERTY()
	UTutorialModuleDataAsset* AuthEssentialsModule;

	UPROPERTY()
	UCommonActivatableWidget* ParentWidget;

	UPROPERTY()
	ULoginWidget_Starter* LoginWidget;

	UPROPERTY()
	UAuthEssentialsSubsystem_Starter* AuthSubsystem;
};
