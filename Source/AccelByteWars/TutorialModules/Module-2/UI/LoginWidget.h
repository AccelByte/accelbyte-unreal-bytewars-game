// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-2/AuthEssentialsSubsystem.h"
#include "TutorialModules/Module-2/AuthEssentialsModels.h"
#include "LoginWidget.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;
class UAccelByteWarsGameInstance;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API ULoginWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	inline static FOnLoginWithSinglePlatformAuth OnLoginWithSinglePlatformAuthDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

	void SetLoginState(const ELoginState NewState);
	void QuitGame();
	void AutoLoginCmd();

#pragma region Module.2 Function Declarations
	void OnLoginWithDeviceIdButtonClicked();
	void OnRetryLoginButtonClicked();
	void OnQuitGameButtonClicked();

	void Login(EAccelByteLoginType LoginMethod);
	void OnLoginComplete(bool bWasSuccessful, const FString& ErrorMessage);
#pragma endregion

#pragma region Module.4 Function Declarations
	void OnLoginWithSinglePlatformAuth();
#pragma endregion

private:
	UAccelByteWarsGameInstance* GameInstance;
	UAuthEssentialsSubsystem* AuthSubsystem;
	UPromptSubsystem* PromptSubsystem;

	EAccelByteLoginType LastLoginMethod = EAccelByteLoginType::None;

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