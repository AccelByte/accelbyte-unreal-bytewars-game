// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "FTUEDialogueWidget.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogFTUEDialogueWidget, Log, All);
#define UE_LOG_FTUEDIALOGUEWIDGET(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogFTUEDialogueWidget, Verbosity, Format, ##__VA_ARGS__) \
}

class UAccelByteWarsGameInstance;
class UAccelByteWarsButtonBase;
class UTextBlock;
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

	// Close FTUE dialogue but not reset from start.
	void PauseDialogues();

	// Show FTUE dialogue from last shown dialogue.
	void ResumeDialogues();

	// Go to next dialogue. Only work if the FTUE is shown.
	void PrevDialogue();

	// Go to previous dialogue. Only work if the FTUE is shown.
	void NextDialogue();

	// Toggle show/hide help button. 
	// If the FTUE is hidden or no FTUE dialogues to be shown, it will force to hide the help button.
	void TryToggleHelpDev(bool bShow);

protected:
	virtual void NativeConstruct() override;

	bool InitializeDialogue(FFTUEDialogueModel* Dialogue);
	void InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel);

	void ValidateDialogues();
	void DeinitializeLastDialogue();
	void ClearHighlightedWidget();

	bool IsWidgetVisible(UWidget* Widget);

	UPROPERTY(EditDefaultsOnly)
	bool bIsAbleToNavigate = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCanvasPanel* Canvas_FTUE;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUEDialogue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUEInterupter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_Message;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Open;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Prev;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Next;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action1;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action2;

	UAccelByteWarsGameInstance* GameInstance;

private:
	bool IsAllDialoguesAlreadyShown();

	// Helper as cursor to navigate between dialogues.
	int32 DialogueIndex = INDEX_NONE;

	// List of dialogues used to display FTUE in this widget. Intended to be modified.
	TArray<FFTUEDialogueModel*> DialoguesInternal;

	// The original list of dialogues injected by other UIs without modification.
	TArray<FFTUEDialogueModel*> DialoguesOrigin;

	FFTUEDialogueModel* CachedLastDialogue;
	UUserWidget* CachedHighlightedWidget;
};
