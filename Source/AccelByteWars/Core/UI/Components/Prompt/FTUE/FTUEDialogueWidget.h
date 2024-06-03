// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "FTUEDialogueWidget.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogFTUEDialogueWidget, Log, All);
#define UE_LOG_FTUEDIALOGUEWIDGET(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogFTUEDialogueWidget, Verbosity, Format, ##__VA_ARGS__) \
}

class UAccelByteWarsGameInstance;
class UAccelByteWarsButtonBase;
class UButton;
class UTextBlock;
class URichTextBlock;
class UCanvasPanel;

UCLASS()
class ACCELBYTEWARS_API UFTUEDialogueWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void AddDialogues(const TArray<FFTUEDialogueModel*>& Dialogues);
	bool RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	// Show FTUE dialogue from start.
	void ShowDialogues(bool bFirstTime = false);

	// Close FTUE dialogue and reset from start.
	void CloseDialogues();

	// Reset FTUE dialogue from start.
	void ResetDialogues();

	// Close FTUE dialogue but not reset from start.
	void PauseDialogues() const;

	// Show FTUE dialogue from last shown dialogue.
	void ResumeDialogues();

	// Go to next dialogue. Only work if the FTUE is shown.
	void PrevDialogue();

	// Go to previous dialogue. Only work if the FTUE is shown.
	void NextDialogue();

	// Go to specific dialogue. Only work if the FTUE is shown.
	void JumpToDialogue(const uint_fast8_t TargetIndex);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool NativeOnHandleBackAction() override;

	bool InitializeDialogue(FFTUEDialogueModel* Dialogue);
	void InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel);
	void InitializeNextButton(const FFTUEDialogueModel* Dialogue) const;
	void InitializePrevButton(const FFTUEDialogueModel* Dialogue) const;
	void InitializeCloseButton() const;
	void InitializePageNumber() const;

	void ValidateDialogues();
	void OnValidateDialoguesComplete();

	void ValidateDialogue(const int32 DialogueToValidateIndex = INDEX_NONE);
	void OnValidateDialogueComplete(FFTUEDialogueModel* Dialogue, const bool bIsValid, const int32 NextDialogueToValidateIndex = INDEX_NONE);

	void DeinitializeLastDialogue() const;
	void ClearHighlightedWidget();

	void CloseDialoguesByGroup();

	void HandleDarkBorderTransition(
		const FFTUEDialogueModel* CurrentDialogue, 
		const FFTUEDialogueModel* TargetDialogue, 
		const TFunction<void()>& OnComplete = {}) const;

	UFUNCTION()
	void OnInterrupterClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	UPROPERTY(EditDefaultsOnly)
	bool bIsAbleToNavigate = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCanvasPanel* Canvas_FTUE;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUEDialogue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUEInterrupter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_DarkBorder;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	URichTextBlock* Tb_Message;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_PageNumber;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Close;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Prev;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Next;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action1;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataTableRowHandle CloseButtonInputActionData;

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance;

	UPROPERTY()
	UAccelByteWarsBaseUI* BaseUIWidget;

private:
	bool IsAllDialoguesAlreadyShown() const;
	static bool IsDialoguePositionEqual(const FFTUEDialogueModel* Dialogue, const FFTUEDialogueModel* TargetDialogue);

	static void PlayFadeInAnimation(
		UWidget* TargetWidget,
		const float AnimationSpeedModifier = 1.0f,
		const TFunction<void()>& OnAnimationCompleted = {},
		const bool bEnableShrinkIn = true);

	static void PlayFadeOutAnimation(
		UWidget* TargetWidget,
		const float AnimationSpeedModifier = 1.0f,
		const TFunction<void()>& OnAnimationCompleted = {},
		const bool bEnableExpandOut = true);

	static void PlayZoomInOutAnimation(
		UWidget* TargetWidget,
		const float AnimationSpeedModifier = 1.0f,
		const TFunction<void()>& OnAnimationCompleted = {},
		const int BounceCount = 2);

	static void SetWidgetOpacityAndTransform(
		UWidget* TargetWidget,
		const float Opacity,
		const FWidgetTransform& Transform);

	// Helper as cursor to navigate between dialogues.
	int32 DialogueIndex = INDEX_NONE;

	// List of dialogues used to display FTUE in this widget. Intended to be modified.
	TArray<FFTUEDialogueModel*> DialoguesInternal;
	TMap<FFTUEDialogueGroup*, TArray<FFTUEDialogueModel*>> DialoguesByGroup;

	// The original list of dialogues injected by other UIs without modification.
	TArray<FFTUEDialogueModel*> DialoguesOrigin;

	FFTUEDialogueModel* CachedLastDialogue;

	UPROPERTY()
	UUserWidget* CachedHighlightedWidget;

	// Helper to cache whether the dialogues run in first time mode.
	bool bIsFirstTimeMode = false;

	// Helper to cache whether the dialogues are already validated or not.
	bool bIsAllDialogueValidated = false;
};
