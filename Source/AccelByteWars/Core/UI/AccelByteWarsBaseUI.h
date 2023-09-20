// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/BackgroundBlur.h"
#include "Components/Prompt/Loading/LoadingWidget.h"
#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "AccelByteWarsBaseUI.generated.h"

class UPopUpWidget;
class UFTUEDialogueWidget;
class AccelByteWarsActivatableWidget;

UENUM()
enum EBaseUIStackType
{
	Prompt UMETA(DisplayName = "Prompt"),
	Menu UMETA(DisplayName = "Menu"),
	InGameMenu UMETA(DisplayName = "In-Game Menu"),
	InGameHUD UMETA(DisplayName = "In-Game HUD")
};

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBaseUI : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable)
	void ToggleBackgroundBlur(const bool bShow) const;

	static UCommonActivatableWidget* GetActiveWidgetOfStack(const EBaseUIStackType TargetStack, const UObject* Context);

	TArray<UCommonActivatableWidget*> GetAllWidgetsBelowStacks(const EBaseUIStackType CurrentStack);

	EBaseUIStackType GetTopMostActiveStack();

	/** Push widget to target stack. */
	UFUNCTION(BlueprintCallable)
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	/** Push widget to target stack with init function. */
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, TFunctionRef<void(UAccelByteWarsActivatableWidget&)> InitFunc);

	UPushNotificationWidget* GetPushNotificationWidget();

	UFTUEDialogueWidget* GetFTUEDialogueWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Base UI Settings")
	TMap<TEnumAsByte<EBaseUIStackType>, UCommonActivatableWidgetStack*> Stacks;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UPopUpWidget> DefaultPopUpWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<ULoadingWidget> DefaultLoadingWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UPushNotificationWidget> DefaultPushNotificationWidgetClass;

private:
	void OnWidgetTransitionChanged(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBackgroundBlur* BackgroundBlur;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* InGameHUDStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* InGameMenuStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* MenuStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* PromptStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetStack* PushNotificationStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UFTUEDialogueWidget* W_FTUEDialogue;
};