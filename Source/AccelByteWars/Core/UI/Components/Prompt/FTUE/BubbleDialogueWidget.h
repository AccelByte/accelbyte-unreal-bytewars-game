// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "BubbleDialogueWidget.generated.h"

class UAccelByteWarsButtonBase;
class UTextBlock;
class UCanvasPanel;

UCLASS()
class ACCELBYTEWARS_API UBubbleDialogueWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void AddDialogues(TArray<FFTUEDialogueModel> Dialogues);
	void RemoveAssociateDialogues(const UTutorialModuleDataAsset* TutorialModule);

	void ShowDialogues();
	void CloseDialogues();

	void NextDialogue();
	void PrevDialogue();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void InitializeDialogue(const FFTUEDialogueModel& Dialogue);
	void InitializeActionButton(UAccelByteWarsButtonBase* Button, const FFTUEDialogueButtonModel& ButtonModel);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCanvasPanel* Canvas_FTUE;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUE;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FTUEInterupter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Txt_Message;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Open;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Close;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action1;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Action2;

private:
	TArray<FFTUEDialogueModel> CachedDialogues;
	int32 DialogueIndex;

	UUserWidget* CachedHighlightedWidget;
};
