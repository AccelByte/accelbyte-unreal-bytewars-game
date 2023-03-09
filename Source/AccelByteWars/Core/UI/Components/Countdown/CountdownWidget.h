// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "CountdownWidget.generated.h"

UENUM(BlueprintType)
enum class ECountdownState : uint8
{
	PRE = 0,
	COUNTING,
	POST,
	INVALID
};

DECLARE_DELEGATE_RetVal(ECountdownState, FCheckCountdownState);
DECLARE_DELEGATE_RetVal(int, FUpdateCountdownValue);
DECLARE_MULTICAST_DELEGATE(FOnCountdownFinished);

class AAccelByteWarsGameStateBase;
/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UCountdownWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	UCountdownWidget(const FObjectInitializer& ObjectInitializer);

	//~UUserWidget overriden functions
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnActivated() override;
	//~End of UUserWidget overriden functions

public:
	/**
	 * @brief Change texts that will be displayed throughout the countdown's states
	 * @param PreCountdownText Text displayed before the counting happened
	 * @param PostCountdownText Text displayed after the counting finished
	 * @param CountdownText Text displayed when the counting happened | set this to empty will collapsed this text
	 * @param bInForceTick If true, tick will still be called even when counting state is POST.
	 */
	void SetupWidget(
		const FText PreCountdownText,
		const FText PostCountdownText,
		const FText CountdownText = FText(),
		const bool bInForceTick = false);

	/**
	 * @brief Called on tick to determine current countdown state
	 * When this returns ECountdownState::Post, this widget's visibility will be set to collapsed, disabling the tick
	 */
	FCheckCountdownState CheckCountdownStateDelegate;

	/**
	 * @brief Called on tick to determine current countdown count.
	 * Will only be called when CheckCountdownStateDelegate returns ECountdownState::Counting
	 */
	FUpdateCountdownValue UpdateCountdownValueDelegate;

	/**
	 * @brief Called once the moment CheckCountdownStateDelegate returns ECountdownState::Post
	 */
	FOnCountdownFinished OnCountdownFinishedDelegate;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* Widget_Outer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* WidgetSwitcher_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_PreCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_PostCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_CountdownValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_CountdownDescription;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* Panel_Countdown;

	UPROPERTY()
	AAccelByteWarsGameStateBase* ByteWarsGameState;

	void CollapseWidgetWithTimer();
	void ChangeWidgetVisibility(const bool bVisible);

	FTimerHandle CollapseWidgetTimer;
	const float ClosingDuration = 1.0f;
	float ClosingElapsed = 0.0f;
	bool bHasFinished = false;
	bool bClosing = false;
	bool bForceTick = false;
};
