// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem_Starter.h"
#include "TutorialModules/Module-2/AuthEssentialsModels.h"
#include "LoginWidget_Starter.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API ULoginWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
			
protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void SetLoginState(const ELoginState NewState);
	
	void OnLoginWithDeviceIdButtonClicked();
	void OnRetryLoginButtonClicked();
	void OnQuitGameButtonClicked();
	
	void QuitGame();

private:
	UAccelByteWarsGameInstance* GameInstance;
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