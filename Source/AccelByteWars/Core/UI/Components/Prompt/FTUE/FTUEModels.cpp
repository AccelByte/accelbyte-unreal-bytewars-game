// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

void FFTUEDialogueModel::ExecuteValidation(const FOnFTUEDialogueValidationComplete& OnComplete, const UObject* Context)
{
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
