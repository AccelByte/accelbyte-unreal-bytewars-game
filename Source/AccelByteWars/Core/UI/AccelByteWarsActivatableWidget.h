// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Core/UI/AccelByteWarsWidgetModels.h"
#include "Core/UI/AccelByteWarsWidgetInterface.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "AccelByteWarsActivatableWidget.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsActivatableWidget, Log, All);
#define UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogAccelByteWarsActivatableWidget, Verbosity, Format, ##__VA_ARGS__) \
}

class UTutorialModuleDataAsset;
class UAccelByteWarsButtonBase;
class UPanelWidget;
class UAccelByteWarsTabListWidget;

UCLASS(Abstract, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsActivatableWidget : public UCommonActivatableWidget, public IAccelByteWarsWidgetInterface
{
	GENERATED_BODY()

public:
	UAccelByteWarsActivatableWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativePreConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

	// Set the value which base widget UI stack where this widget is spawned.
	void SetWidgetStackType(const EBaseUIStackType StackType);

	// Get the value which base widget UI stack where this widget is spawned.
	EBaseUIStackType GetWidgetStackType() const;

	// Set whether to allow the UI to move the camera location or not.
	void AllowMoveCamera(bool bAllow);
	
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

	/** Get the first outer with the specified class. Nullptr if not found. */
	template<class WidgetClass>
	WidgetClass* GetFirstOccurenceOuter()
	{
		UObject* CurrentIterationOuter = GetOuter();
		WidgetClass* TargetOuter = nullptr;

		while (!TargetOuter)
		{
			if (!CurrentIterationOuter)
			{
				break;
			}

			TargetOuter = Cast<WidgetClass>(CurrentIterationOuter);
			CurrentIterationOuter = CurrentIterationOuter->GetOuter();
		}

		return TargetOuter;
	}

	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EAccelByteWarsWidgetInputMode InputConfig = EAccelByteWarsWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

	// The base widget UI stack where this widget is spawned.
	EBaseUIStackType WidgetStackType = EBaseUIStackType::Menu;

	// Whether to allow the UI to move the camera location or not. 
	bool bAllowMoveCamera = true;

#pragma region "AccelByte SDK Config Menu"
	/** Handle to store action binding to open AccelByte SDK reconfiguration menu. */
	FUIActionBindingHandle OpenSdkConfigHandle;
#pragma endregion

#pragma region "Tutorial Module"
public:
	/** The Tutorial Module Data Asset associated with this widget. */
	UPROPERTY()
	UTutorialModuleDataAsset* AssociateTutorialModule;

private:
	void ValidateAssociateTutorialModule();
#pragma endregion

#pragma region "Generated Widgets"
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Module Metadata")
	TArray<UPanelWidget*> GetGeneratedWidgetContainers();

	/**
	 * Will only be used for OTHER_TUTORIAL_MODULE_ENTRY_TABLIST and TUTORIAL_MODULE_ENTRY_TABLIST
	 * @brief Implement this if your module need to use OTHER_TUTORIAL_MODULE_ENTRY_TABLIST or TUTORIAL_MODULE_ENTRY_TABLIST. Otherwise, implement GetGeneratedWidgetContainers instead. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Module Metadata")
	TArray<UAccelByteWarsTabListWidget*> GetGeneratedWidgetTabListContainers();

	bool GetIsAllGeneratedWidgetsShouldNotDisplay() const;

	/** The generated widget metadatas injected by one or more Tutorial Modules. */
	TArray<FTutorialModuleGeneratedWidget*> GeneratedWidgets;

protected:
	void InitializeGeneratedWidgets();

	/** If set to true, hide this UI if the generated widgets is empty */
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Module Metadata")
	bool bHideIfGeneratedWidgetEmpty = false;

private:
	void ValidateGeneratedWidgets();
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateEntryButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateActionButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsActivatableWidget> GenerateWidget(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateTabListEntryButton(
		FTutorialModuleGeneratedWidget& Metadata,
		UAccelByteWarsTabListWidget& WidgetContainer);

	TSubclassOf<UAccelByteWarsActivatableWidget> GetValidEntryWidgetClass(const FTutorialModuleGeneratedWidget& Metadata) const;

	UPROPERTY()
	TArray<UUserWidget*> GeneratedWidgetPool;
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
public:
	/** The FTUE dialogues to be shown when this widget is active. */
	TArray<FFTUEDialogueModel*> FTUEDialogues;
	
	/** Open FTUE Menu Widget from in game state */
	void OpenFTUEWidget() const;

	/** Hide the FTUE Dev Help Button */
	void HideFTUEDevHelpButton(const bool bInHideButton = true) const;

	/** Hide the FTUE Dev Help Input Action */
	void HideFTUEDevHelpInputAction(const bool bInHideInputAction = true) const;

protected:
	void InitializeFTUEDialogues(bool bShowOnInitialize, bool bIsFirstTime = true) const;
	void DeinitializeFTUEDialogues() const;
	mutable FOnValidateDialoguesComplete CreateFTUEWidgetDelegate;

	UPROPERTY(EditAnywhere, Category = "Tutorial Module Metadata", meta = (ToolTip = "Whether this widget should initialize FTUE on activated."))
	bool bOnActivatedInitializeFTUE = true;

	bool bLoadFTUEImmediately = true;
private:
	/** Generate FTUE DevHelpButton */
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateDevHelpButton();
	TWeakObjectPtr<UAccelByteWarsButtonBase> GetDevHelpButton() const;
	void DestroyDevHelpButton() const;

	void ValidateFTUEDialogues();
	void ValidateDevHelpButton();
	void OnValidateDialogueComplete(FFTUEDialogueModel* Dialogue, const bool bIsValid);
#pragma endregion

#pragma region "Widget Validators"
public:
	void ExecuteWidgetValidators();

	// The widget validators to be executed when this widget is active.
	TArray<FWidgetValidator*> WidgetValidators;

private:
	void ValidateWidgetValidators();
#pragma endregion
};