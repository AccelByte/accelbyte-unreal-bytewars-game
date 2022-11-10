// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AccelByteWarsActivatableWidget.generated.h"


UENUM(BlueprintType)
enum class EAccelByteWarsWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer);
	
public:

	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
		EAccelByteWarsWidgetInputMode InputConfig = EAccelByteWarsWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
		EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
	
};
