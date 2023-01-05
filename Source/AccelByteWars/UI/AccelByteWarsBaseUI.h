// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "AccelByteWarsBaseUI.generated.h"

UENUM()
enum EBaseUIStackType
{
	Menu,
	Prompt,
	InGameMenu,
	InGameHUD
};

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBaseUI : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	/** Push widget to target stack. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Push Widget to Stack"))
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	/** Menu stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* MenuStack;

	/** Prompt stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* PromptStack;

	/** In game menu stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* InGameMenuStack;

	/** In game HUD stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* InGameHUDStack;
};