// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "WidgetValidatorModels.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogWidgetValidator, Log, All);

#define UE_LOG_WIDGETVALIDATOR(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogWidgetValidator, Verbosity, Format, ##__VA_ARGS__) \
}

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"
#define DEFAULT_WIDGET_VALIDATOR_VALIDATING_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Default Widget Validator Validating State Message", "Validating")
#define DEFAULT_WIDGET_VALIDATOR_INVALID_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Default Widget Validator Invalid State Message", "Invalid Validation")
#define DEFAULT_WIDGET_VALIDATOR_TIMEOUT_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Default Widget Validator Timeout Message", "Validation Timeout")

class UUserWidget;
class UCommonUserWidget;
class UTutorialModuleDataAsset;
class UAccelByteWarsActivatableWidget;
class IAccelByteWarsWidgetInterface;

USTRUCT(BlueprintType)
struct FWidgetValidator
{
    GENERATED_BODY()

public:
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        meta = (
            Tooltip = "The Tutorial Module who owns the metadata of the widget validator.",
            DisplayThumbnail = false))
    UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "(Optional) The id to identify the widget validator."))
    FString WidgetValidatorId = FString("");

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "Target widget container where the validator should be executed."))
    TArray<TSubclassOf<UAccelByteWarsActivatableWidget>> TargetWidgetContainerClasses;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "The target widget name to validate."))
    FString TargetWidgetNameToValidate = FString("");

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "The class type of the widget to be validated."))
    TSubclassOf<UCommonUserWidget> TargetWidgetClassToValidate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Tooltip = "Validation to perform. If Custom Validation selected, you need to implement it on your own through code."))
    FServicePredefinedValidator Validator;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "Default message to be shown when the validator is in invalid state."))
    FString DefaultInvalidStateMessage = DEFAULT_WIDGET_VALIDATOR_INVALID_MESSAGE.ToString();

    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        meta = (Tooltip = "Service URL to be opened when the validator is invalid."))
    FString ServiceURL = FString("");

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (Tooltip = "Whether the URL should be formatted or not."))
    bool bIsFormattedURL = false;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (
            Tooltip = "List of argument to be inserted on the formatted URL. The order and the size of the arguments must follow the formatted URL.",
            EditCondition = "bIsFormattedURL",
            EditConditionHides))
    TArray<FServiceArgumentModel> URLArguments;

    // Delegate to be executed when the widget validation is started.
    TDelegate<void(FWidgetValidator* /*WidgetValidator*/)> OnValidatorExecutedDelegate;

    // Delegate to be executed when the widget validation is completed.
    TDelegate<void(FWidgetValidator* /*WidgetValidator*/, const bool /*bIsValidState*/, const FString& /*InvalidStateMessage*/)> OnValidatorFinalizedDelegate;

    // Event to invoke predefined service valdator.
    inline static TDelegate<void(FWidgetValidator* /*WidgetValidator*/, const UObject* Context)> OnPredefinedServiceValidatorExecutedDelegate;

    // Event to be called when the widget validation is started.
    void ExecuteValidator(IAccelByteWarsWidgetInterface* Widget, const UObject* Context, const bool bForceValidate = false);

    // Event to be called when the widget validation is completed.
    void FinalizeValidator(const bool bIsValidState, const FString& InvalidStateMessage = FString(""), const bool bFallbackToDefaultInvalidMessage = true);

    FString GetFormattedURL() const
    {
        if (!bIsFormattedURL)
        {
            return ServiceURL;
        }

        FFormatNamedArguments Args = FFormatNamedArguments();
        const int32 ArgsNum = URLArguments.Num();
        for (int32 i = 0; i < ArgsNum; i++)
        {
            const FServiceArgumentModel Arg = URLArguments[i];

            FString ArgStr = FString("");
            if (Arg.bUsePredefinedArgument && Arg.OnGetPredefinedArgument.IsBound())
            {
                ArgStr = Arg.OnGetPredefinedArgument.Execute(Arg.PredefinedArgument);
            }
            else
            {
                ArgStr = Arg.Argument;
            }

            Args.Add(FString::Printf(TEXT("%d"), i), FText::FromString(ArgStr));
        }

        return FText::Format(FText::FromString(ServiceURL), Args).ToString();
    }

    static FWidgetValidator* GetMetadataById(const FString& WidgetValidatorId);

    IAccelByteWarsWidgetInterface* GetCachedWidget() const
    {
        return CachedWidget;
    }

private:
    IAccelByteWarsWidgetInterface* CachedWidget = nullptr;

    bool bIsFinalized = false;

    float ValidationTimeout = 10.0f;
    bool bIsTimeout = false;

    void OnValidatorTimeout();
};