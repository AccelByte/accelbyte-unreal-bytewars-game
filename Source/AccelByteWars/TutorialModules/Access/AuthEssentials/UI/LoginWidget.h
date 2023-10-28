// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "LoginWidget.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULoginWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
			
public:
	void SetLoginState(const ELoginState NewState);
	void OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage);

	FOnRetryLogin OnRetryLoginDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	
	void OnLoginWithDeviceIdButtonClicked();
	void OnRetryLoginButtonClicked();
	void OnQuitGameButtonClicked();
	
	void AutoLoginCmd();

private:
	UAccelByteWarsGameInstance* GameInstance;
	UAuthEssentialsSubsystem* AuthSubsystem;
	UPromptSubsystem* PromptSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_LoginState;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_LoginWithDeviceId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_RetryLogin;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_QuitGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;
};