// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "AccelByteWarsActivatableWidget.generated.h"


UENUM(BlueprintType)
enum class EAccelByteWarsWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

class UTutorialModuleDataAsset;
class UAccelByteWarsButtonBase;
class UPanelWidget;

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativePreConstruct() override;
	virtual void NativeOnActivated() override;

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Module Metadata")
	TArray<UPanelWidget*> GetGeneratedWidgetContainers();

	// The Tutorial Module Data Asset associated with this widget.
	UTutorialModuleDataAsset* AssociateTutorialModule;

	// The generated widget metadatas injected by one or more Tutorial Modules.
	TArray<FTutorialModuleGeneratedWidget*> GeneratedWidgets;

protected:
	/** Change the owning player controller input mode to game only and also hide the mouse cursor */
	UFUNCTION(BlueprintCallable)
	void MoveCameraToTargetLocation(const float DeltaTime, const FVector TargetLocation = FVector(60.0f, 300.0f, 160.0f), const float InterpSpeed = 5.0f);

	/** Change the owning player controller input mode to UI only and also show the mouse cursor */
	UFUNCTION(BlueprintCallable)
	void SetInputModeToUIOnly();

	/** Change the owning player controller input mode to game only and also hide the mouse cursor */
	UFUNCTION(BlueprintCallable)
	void SetInputModeToGameOnly();

	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EAccelByteWarsWidgetInputMode InputConfig = EAccelByteWarsWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

private:
	void ValidateGeneratedWidgets();
	void InitializeGeneratedWidgets();
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateEntryButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateActionButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsActivatableWidget> GenerateWidget(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);

	TArray<UUserWidget*> GeneratedWidgetPool;
};