// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "WidgetValidatorModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/AccelByteWarsWidgetInterface.h"

DEFINE_LOG_CATEGORY(LogWidgetValidator);

void FWidgetValidator::ExecuteValidator(IAccelByteWarsWidgetInterface* Widget, const UObject* Context, const bool bForceValidate)
{
    if (!Widget || !Widget->_getUObject())
    {
        UE_LOG_WIDGETVALIDATOR(Warning, TEXT("Unable to execute widget validator. Target widget to validate is null."));
        return;
    }

    // To prevent multiple validation calls, same instance of target widget can only be validated once (if not forced).
    if (CachedWidget && CachedWidget->_getUObject())
    {
        const bool bSameTargetWidget = CachedWidget == Widget || CachedWidget->_getUObject() == Widget->_getUObject();
        if (!bForceValidate && bSameTargetWidget)
        {
            UE_LOG_WIDGETVALIDATOR(Warning, TEXT("Unable to execute widget validator. Same target widget is already or being validated."));
            return;
        }
    }
    
    bIsTimeout = false;
    bIsFinalized = false;

    // Execute widget validator.
    CachedWidget = Widget;

    Widget->Execute_SetWidgetValidationState(
        Widget->_getUObject(),
        EWidgetValidationState::VALIDATING,
        DEFAULT_WIDGET_VALIDATOR_VALIDATING_MESSAGE.ToString(),
        GetFormattedURL());
    
    const EServicePredefinedValidator ValidatorType = Validator.ValidatorType;
    if (ValidatorType == EServicePredefinedValidator::NONE) 
    {
        FinalizeValidator(true);
        return;
    }
    else if (ValidatorType == EServicePredefinedValidator::IS_CUSTOM)
    {
        OnValidatorExecutedDelegate.ExecuteIfBound(this);
    }
    else
    {
        OnPredefinedServiceValidatorExecutedDelegate.ExecuteIfBound(this, Context);
    }

    // Start widget validator time out.
    Async(EAsyncExecution::TaskGraph, [this]
    {
        FPlatformProcess::Sleep(ValidationTimeout);
        AsyncTask(ENamedThreads::GameThread, [this]
        {
            OnValidatorTimeout();
        });
    });
}

void FWidgetValidator::FinalizeValidator(const bool bIsValidState, const FString& InvalidStateMessage, const bool bFallbackToDefaultInvalidMessage)
{
    if (bIsTimeout) 
    {
        UE_LOG_WIDGETVALIDATOR(Warning, TEXT("Unable to finalize widget validator. Validation timeout."));
        return;
    }

    if (!CachedWidget || !CachedWidget->_getUObject())
    {
        UE_LOG_WIDGETVALIDATOR(Warning, TEXT("Unable to finalize widget validator. Target widget to validate is null."));
        return;
    }

    bIsFinalized = true;

    FString FinalInvalidStateMessage = FString();
    if (!bIsValidState)
    {
        FinalInvalidStateMessage = (InvalidStateMessage.IsEmpty() && bFallbackToDefaultInvalidMessage) ? DefaultInvalidStateMessage : InvalidStateMessage;
    }

    const EWidgetValidationState FinalValidationState = bIsValidState ? EWidgetValidationState::VALID : EWidgetValidationState::INVALID;
    CachedWidget->Execute_SetWidgetValidationState(
        CachedWidget->_getUObject(),
        FinalValidationState,
        FinalInvalidStateMessage,
        GetFormattedURL());

    OnValidatorFinalizedDelegate.ExecuteIfBound(this, bIsValidState, FinalInvalidStateMessage);
}

void FWidgetValidator::OnValidatorTimeout()
{
    // Abort if validation is already finalized.
    if (bIsFinalized) 
    {
        return;
    }

    if (!CachedWidget || !CachedWidget->_getUObject())
    {
        UE_LOG_WIDGETVALIDATOR(Warning, TEXT("Unable to handle validation timeout. Target widget to validate is null."));
        return;
    }

    bIsTimeout = true;

    CachedWidget->Execute_SetWidgetValidationState(
        CachedWidget->_getUObject(),
        EWidgetValidationState::INVALID,
        DEFAULT_WIDGET_VALIDATOR_TIMEOUT_MESSAGE.ToString(),
        GetFormattedURL());
}

FWidgetValidator* FWidgetValidator::GetMetadataById(const FString& WidgetValidatorId)
{
    FWidgetValidator* Result = nullptr;

    for (const auto Metadata : UTutorialModuleDataAsset::GetCachedWidgetValidators())
    {
        if (!Metadata)
        {
            continue;
        }

        if (Metadata->WidgetValidatorId.Equals(WidgetValidatorId))
        {
            Result = Metadata;
            break;
        }
    }

    return Result;
}