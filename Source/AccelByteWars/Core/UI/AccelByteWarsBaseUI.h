// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "AccelByteWarsBaseUI.generated.h"

class UPopUpWidget;

UENUM()
enum EBaseUIStackType
{
	Prompt UMETA(DisplayName = "Prompt"),
	Menu UMETA(DisplayName = "Menu"),
	InGameMenu UMETA(DisplayName = "In-Game Menu"),
	InGameHUD UMETA(DisplayName = "In-Game HUD")
};

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBaseUI : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void NativeOnInitialized() override;

	/** Push widget to target stack. */
	UFUNCTION(BlueprintCallable)
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	/** Push widget to target stack with init function. */
	UAccelByteWarsActivatableWidget* PushWidgetToStack(EBaseUIStackType TargetStack, TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass, TFunctionRef<void(UAccelByteWarsActivatableWidget&)> InitFunc);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Base UI Settings")
	TMap<TEnumAsByte<EBaseUIStackType>, UCommonActivatableWidgetStack*> Stacks;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UPopUpWidget> DefaultPopUpWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Prompt Settings")
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultLoadingWidgetClass;

private:
	void OnWidgetTransitionChanged(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);
};