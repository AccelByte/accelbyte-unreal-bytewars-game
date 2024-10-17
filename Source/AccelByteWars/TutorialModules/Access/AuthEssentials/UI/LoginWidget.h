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

// @@@SNIPSTART LoginWidget.h-public
// @@@MULTISNIP Ws_LoginState {"selectedLines": ["1", "6-7"]}
public:
	void SetLoginState(const ELoginState NewState) const;
	void OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage);
	void SetButtonLoginVisibility(const ESlateVisibility InSlateVisibility) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_LoginState;

	FOnRetryLogin OnRetryLoginDelegate;
// @@@SNIPEND

// @@@SNIPSTART LoginWidget.h-protected
protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	
	void OnLoginWithDeviceIdButtonClicked();
	void OnRetryLoginButtonClicked();
	void OnQuitGameButtonClicked();
	
	void OpenMainMenu();

	void AutoLoginCmd();
// @@@SNIPEND

// @@@SNIPSTART LoginWidget.h-private
// @@@MULTISNIP DefaultStateUI {"selectedLines": ["1", "11-12"]}
// @@@MULTISNIP FailedStateUI {"selectedLines": ["1", "14-21"]}
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* Vb_LoginOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* Vb_LoginLoading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* Vb_LoginFailed;

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

	UPROPERTY(EditDefaultsOnly)
	FPrimaryAssetId RegisterUserInGameAssetId;

	UAccelByteWarsGameInstance* GameInstance;
	UAuthEssentialsSubsystem* AuthSubsystem;
	UPromptSubsystem* PromptSubsystem;
// @@@SNIPEND
};