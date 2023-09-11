// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Core/UI/AccelByteWarsWidgetInterface.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
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

#pragma region "Tutorial Module"
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Module Metadata")
	TArray<UPanelWidget*> GetGeneratedWidgetContainers();

	// The Tutorial Module Data Asset associated with this widget.
	UPROPERTY()
	UTutorialModuleDataAsset* AssociateTutorialModule;

	// The generated widget metadatas injected by one or more Tutorial Modules.
	TArray<FTutorialModuleGeneratedWidget*> GeneratedWidgets;

	// The FTUE dialogues to be shown when this widget is active.
	TArray<FFTUEDialogueModel> FTUEDialogues;
#pragma endregion

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

private:
#pragma region "Tutorial Module"
	void ValidateAssociateTutorialModule();
#pragma endregion

#pragma region "Generated Widgets"
	void ValidateGeneratedWidgets();
	void InitializeGeneratedWidgets();
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateEntryButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsButtonBase> GenerateActionButton(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);
	TWeakObjectPtr<UAccelByteWarsActivatableWidget> GenerateWidget(FTutorialModuleGeneratedWidget& Metadata, UPanelWidget& WidgetContainer);

	UPROPERTY()
	TArray<UUserWidget*> GeneratedWidgetPool;
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
	void ValidateFTUEDialogues();
	void InitializeFTEUDialogues();
	void DeinitializeFTUEDialogues();
#pragma endregion
};