// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

TMap<const UObject*, TArray<FFTUEDialogueModel*>> FFTUEDialogueModel::DialoguesToValidate;
TArray<const UObject*> FFTUEDialogueModel::ValidationInterruptRequest;

void FFTUEDialogueModel::ExecuteValidation(const FOnFTUEDialogueValidationComplete& OnComplete, const UObject* Context)
{
	if(ValidationResult != EValidationResult::NONE)
	{
		// Use cached result if already validated
		OnComplete.ExecuteIfBound(this, ValidationResult == EValidationResult::VALID);
		return;
	}
	
	if (Validator.ValidatorType == EServicePredefinedValidator::NONE)
	{
		OnComplete.ExecuteIfBound(this, true);
	}
	else if (Validator.ValidatorType == EServicePredefinedValidator::IS_CUSTOM)
	{
		OnCustomValidationDelegate.ExecuteIfBound(OnComplete);
	}
	else 
	{
		OnPredefinedValidationDelegate.ExecuteIfBound(this, OnComplete, Context);
	}
}

FFTUEDialogueModel* FFTUEDialogueModel::GetMetadataById(const FString& FTUEId)
{
	FFTUEDialogueModel* Result = nullptr;

	for (const auto Metadata : UTutorialModuleDataAsset::GetCachedFTUEDialogues())
	{
		if (!Metadata)
		{
			continue;
		}

		if (Metadata->FTUEId.Equals(FTUEId))
		{
			Result = Metadata;
			break;
		}
	}

	return Result;
}

void FFTUEDialogueModel::ValidateDialogues(const TArray<FFTUEDialogueModel*>& Dialogues, const UObject* Context,
	FOnValidateDialoguesComplete& OnComplete)
{
	if(ValidationInterruptRequest.Contains(Context))
	{
		// If we have validation interrupt request, cancel it 
		ValidationInterruptRequest.Remove(Context);
		return;
	}
	
	DialoguesToValidate.Add(Context, Dialogues);

	// reset validation result
	for(FFTUEDialogueModel* Dialogue : Dialogues)
	{
		Dialogue->ValidationResult = EValidationResult::NONE;
	}
	ValidateDialogue(DialoguesToValidate[Context], Context, OnComplete, 0);
}

void FFTUEDialogueModel::ValidateDialogue(const TArray<FFTUEDialogueModel*>& Dialogues, const UObject* Context, FOnValidateDialoguesComplete& OnComplete, int32 Index)
{
	bool bInterrupted = ValidationInterruptRequest.Contains(Context);
	if(Index >= Dialogues.Num() || bInterrupted)
	{
		DialoguesToValidate.Remove(Context);
		if(bInterrupted)
		{
			ValidationInterruptRequest.Remove(Context);
		}
		OnComplete.ExecuteIfBound(bInterrupted);
		return;
	}
	
	Dialogues[Index]->ExecuteValidation(FOnFTUEDialogueValidationComplete::CreateLambda([Context, &Dialogues, &OnComplete, Index](FFTUEDialogueModel* CurrentDialog, bool bIsValid)
	{
		CurrentDialog->ValidationResult = bIsValid ? EValidationResult::VALID : EValidationResult::NOT_VALID;
		ValidateDialogue(Dialogues, Context, OnComplete, Index+1);
	}), Context);
}

void FFTUEDialogueModel::TryInterruptValidation(const UObject* Context)
{
	if(DialoguesToValidate.Contains(Context))
	{
		ValidationInterruptRequest.Add(Context);
	}
}
