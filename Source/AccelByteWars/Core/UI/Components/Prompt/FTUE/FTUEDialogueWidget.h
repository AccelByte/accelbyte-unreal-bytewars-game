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

class UAccelByteWarsButtonBase;
class UTextBlock;
class UCanvasPanel;

UCLASS()
class ACCELBYTEWARS_API UFTUEDialogueWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void AddDialogues(TArray<FFTUEDialogueModel>& Dialogues);
	void RemoveAssociateDialogues(const TSubclassOf<UAccelByteWarsActivatableWidget> WidgetClass);

	void ShowDialogues();
	void CloseDialogues();

	void PrevDialogue();
	void NextDialogue();

protected:
	virtual void NativeConstruct() override;

	bool InitializeDialogue(FFTUEDialogueModel* Dialogue);
	void InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel);

	void ValidateDialogues();
	void ClearHighlightedWidget();

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

private:
	TArray<FFTUEDialogueModel*> CachedDialogues;
	int32 DialogueIndex;

	UUserWidget* CachedHighlightedWidget;
};
