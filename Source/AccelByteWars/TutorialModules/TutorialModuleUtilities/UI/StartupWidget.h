// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "StartupWidget.generated.h"

class UWidgetSwitcher;
class UPanelWidget;
class UTextBlock;
class UCommonButtonBase;
class UPromptSubsystem;
class UStartupSubsystem;
class UTutorialModuleDataAsset;
class UAccelByteWarsBaseUI;
class FOnlineErrorAccelByte;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStartupWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void CheckCoreComponents();
	void CheckAutoUseTokenForABLogin();
	
	void StartupGame();
	void QuitGame();

	bool IsLoggedIn();

	void OnLoginPlatformOnlyComplete(const bool bSucceeded, const FString& ErrorMessage);
	void OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error);

	UAccelByteWarsBaseUI* BaseUIWidget;
	UPromptSubsystem* PromptSubsystem;
	UStartupSubsystem* StartupSubsystem;

	FTimerHandle InitTimerHandle;

	static bool bIsInitialized;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InitDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTutorialModuleDataAsset* AuthEssentialsDataAsset;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Startup;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Pw_Init;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Pw_Failed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FailedMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_QuitGame;
};
