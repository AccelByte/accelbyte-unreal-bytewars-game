// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AccelByteWarsActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "AccelByteWarsBaseUI.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBaseUI : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	/** Push stack menu. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Push Menu Widget"))
	void PushToStackMenu(TSubclassOf<UAccelByteWarsActivatableWidget> MenuClass);

	/** Push stack menu. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Push Prompt Widget"))
	void PushToStackPrompt(TSubclassOf<UAccelByteWarsActivatableWidget> PromptClass);

	/** Menu stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* MenuStack;

	/** Prompt stack. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonActivatableWidgetStack* PromptStack;
};
