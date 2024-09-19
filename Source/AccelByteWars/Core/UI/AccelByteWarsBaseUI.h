// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/BackgroundBlur.h"
#include "Components/Prompt/Loading/LoadingWidget.h"
#include "Core/UI/AccelByteWarsWidgetModels.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "AccelByteWarsBaseUI.generated.h"

class UInfoWidget;
class UPopUpWidget;
class UFTUEDialogueWidget;
class AccelByteWarsActivatableWidget;
class UGUICheatWidget;

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBaseUI : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void NativeOnInitialized() override;

	void ClearWidgets();
	void ResetWidget();

	UFUNCTION(BlueprintCallable)
	void ToggleBackgroundBlur(const bool bShow) const;

	UFUNCTION(BlueprintCallable)
	void ToggleProjectInfoWidget(const bool bShow) const;

	static UCommonActivatableWidget* GetActiveWidgetOfStack(const EBaseUIStackType TargetStack, const UObject* Context);

	TArray<UCommonActivatableWidget*> GetAllWidgetsBelowStacks(const EBaseUIStackType CurrentStack);

	EBaseUIStackType GetTopMostActiveStack();

	void AddGUICheatEntry(UGUICheatWidgetEntry* Entry);
	void RemoveGUICheatEntry(UGUICheatWidgetEntry* Entry);
	void RemoveGUICheatEntries(UTutorialModuleDataAsset* TutorialModuleDataAsset);

	/** Push widget to target stack. */
	UFUNCTION(BlueprintCallable)
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	/** Push widget to target stack with init function. */
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, TFunctionRef<void(UAccelByteWarsActivatableWidget&)> InitFunc);

	UPushNotificationWidget* GetPushNotificationWidget();

	UFTUEDialogueWidget* GetFTUEDialogueWidget();

	void SetFTUEDialogueWidget(UFTUEDialogueWidget* InFTUEDialogueWidget);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Base UI Settings")
	TMap<EBaseUIStackType, UCommonActivatableWidgetStack*> Stacks;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UPopUpWidget> DefaultPopUpWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<ULoadingWidget> DefaultLoadingWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UPushNotificationWidget> DefaultPushNotificationWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "GUI Cheat settings")
	TSubclassOf<UGUICheatWidget> DefaultGUICheatWidgetClass;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataTableRowHandle OpenGUICheatInputActionData;

	void ToggleGUICheat();

private:
	bool bStacksCleared = false;

	void OnWidgetTransitionChanged(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	void OnDisplayedWidgetChanged(UCommonActivatableWidget* Widget, const EBaseUIStackType StackType);

	void FocusOnTopMostStack();
	void OverrideFocus(UWidget* Target);
	void RemoveFocusOverride();

	UPROPERTY()
	UWidget* FocusOverride;

	FUIActionBindingHandle OpenGUICheatHandle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBackgroundBlur* BackgroundBlur;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* InGameHUDStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* InGameMenuStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* MenuStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* FTUEStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* PromptStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* PushNotificationStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UGUICheatWidget* W_GUICheat;

	UFTUEDialogueWidget* W_FTUEDialogue;

	UPROPERTY(EditDefaultsOnly, Category = "Project Info UI Settings")
	TArray<TSubclassOf<UAccelByteWarsActivatableWidget>> ProjectInfoTargetWidgets;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UInfoWidget* W_ProjectInfo;
};