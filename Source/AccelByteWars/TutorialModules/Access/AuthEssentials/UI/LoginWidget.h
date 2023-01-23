// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "LoginWidget.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UENUM(BlueprintType)
enum ELoginState 
{
	Default,
	LoggingIn,
	Failed
};

UCLASS()
class ACCELBYTEWARS_API ULoginWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
			
protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void Login(EAccelByteLoginType LoginMethod);
	void SetLoginState(const ELoginState NewState);
	void OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage);

	void QuitGame();

private:
	UAccelByteWarsGameInstance* GameInstance;
	UAuthEssentialsSubsystem* AuthSubsystem;
	UPromptSubsystem* PromptSubsystem;

	EAccelByteLoginType LastLoginMethod;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_LoginState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_LoginWithDeviceId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_LoginWithConsole;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_RetryLogin;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_QuitGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* B_DesktopLogin;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;
};