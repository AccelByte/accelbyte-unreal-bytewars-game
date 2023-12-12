// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"
#include "FTUEModels.generated.h"

class UAccelByteWarsActivatableWidget;
class UCommonUserWidget;
class UTutorialModuleDataAsset;

UENUM(BlueprintType)
enum class EFTUEDialogueHorizontalAnchor : uint8
{
    MIDDLE = 0 UMETA(DisplayName = "Middle"),
    LEFT UMETA(DisplayName = "Left"),
    RIGHT UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EFTUEDialogueVerticalAnchor : uint8
{
    MIDDLE = 0 UMETA(DisplayName = "Middle"),
    TOP UMETA(DisplayName = "Top"),
    BOTTOM UMETA(DisplayName = "Bottom")
};

UENUM(BlueprintType)
enum class EFTUEDialogueButtonActionType : uint8
{
	HYPERLINK_BUTTON = 0 UMETA(DisplayName = "Hyperlink Button"),
    ACTION_BUTTON UMETA(DisplayName = "Custom Action Button")
};

UENUM(BlueprintType)
enum class FFTUEDialogueButtonType : uint8
{
    NO_BUTTON = 0 UMETA(DisplayName = "No Button"),
    ONE_BUTTON UMETA(DisplayName = "One Button"),
    TWO_BUTTONS UMETA(DisplayName = "Two Buttons")
};

UENUM(BlueprintType)
enum class FTUEPredifinedArgument : uint8
{
    PLAYER_ID = 0 UMETA(DisplayName = "Player Id"),
    PLAYER_DISPLAY_NAME UMETA(DisplayName = "Player Display Name"),
    GAME_SESSION_ID UMETA(DisplayName = "Game Session Id"),
    PARTY_SESSION_ID UMETA(DisplayName = "Party Session Id"),
    DEDICATED_SERVER_ID UMETA(DisplayName = "Dedicated Server Id"),
    ENV_BASE_URL UMETA(DisplayName = "Environment Base URL"),
    GAME_NAMESPACE UMETA(DisplayName = "Game Namespace"),
    ADMIN_PORTAL_URL UMETA(DisplayName = "Admin Portal URL")
};

USTRUCT(BlueprintType)
struct FTUEArgumentModel 
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (
        Tooltip = "Whether to use predefined argument or not."))
    bool bUsePredefinedArgument = false;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (
        Tooltip = "User defined string argument.",
        EditCondition = "!bUsePredefinedArgument",
        EditConditionHides))
    FString Argument;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (
        Tooltip = "Predefined argument.",
        EditCondition = "bUsePredefinedArgument",
        EditConditionHides))
    FTUEPredifinedArgument PredefinedArgument = FTUEPredifinedArgument::PLAYER_ID;

    // Event to get predefined argument from online tutorial modules branch.
    inline static TDelegate<FString(const FTUEPredifinedArgument /*Keyword*/)> OnGetPredefinedArgument;
};

USTRUCT(BlueprintType)
struct FFTUEDialogueButtonModel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The action type that should be executed by the button."))
    EFTUEDialogueButtonActionType ButtonActionType = EFTUEDialogueButtonActionType::HYPERLINK_BUTTON;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The text to be shown on the button."))
    FText ButtonText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Whether the button text should be formatted or not.",
        EditConditionHides))
    bool bIsFormattedButtonText = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "List of argument to be inserted on the formatted button text. The order and the size of the arguments must follow the formatted button text.",
        EditCondition = "bIsFormattedButtonText",
        EditConditionHides))
    TArray<FTUEArgumentModel> ButtonTextArguments;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (
        Tooltip = "URL to be opened when the button is clicked.",
        EditCondition = "ButtonActionType==EFTUEDialogueButtonActionType::HYPERLINK_BUTTON", 
        EditConditionHides))
    FString TargetURL;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Whether the URL should be formatted or not.",
        EditCondition = "ButtonActionType==EFTUEDialogueButtonActionType::HYPERLINK_BUTTON",
        EditConditionHides))
    bool bIsFormattedURL = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "List of argument to be inserted on the formatted URL. The order and the size of the arguments must follow the formatted URL.",
        EditCondition = "bIsFormattedURL && ButtonActionType==EFTUEDialogueButtonActionType::HYPERLINK_BUTTON",
        EditConditionHides))
    TArray<FTUEArgumentModel> URLArguments;

    // Action button to be executed if the button action type is a custom action button.
    TMulticastDelegate<void()> ButtonActionDelegate;

    void Reset()
    {
        ButtonActionType = EFTUEDialogueButtonActionType::HYPERLINK_BUTTON;
        ButtonText = FText::GetEmpty();
        TargetURL = FString("");
        bIsFormattedURL = false;
        URLArguments.Empty();
        ButtonActionDelegate.Clear();
    }

    FText GetFormattedButtonText() const
    {
        if (!bIsFormattedButtonText)
        {
            return ButtonText;
        }

        FFormatNamedArguments Args;
        int32 ArgIndex = 0;
        for (auto& Arg : ButtonTextArguments)
        {
            FString ArgStr = FString("");
            if (Arg.bUsePredefinedArgument && Arg.OnGetPredefinedArgument.IsBound())
            {
                ArgStr = Arg.OnGetPredefinedArgument.Execute(Arg.PredefinedArgument);
            }
            else
            {
                ArgStr = Arg.Argument;
            }

            Args.Add(FString::Printf(TEXT("%d"), ArgIndex), FText::FromString(ArgStr));
            ArgIndex++;
        }

        return FText::Format(ButtonText, Args);
    }

    FString GetFormattedURL() const
    {
        if (!bIsFormattedURL)
        {
            return TargetURL;
        }

        FFormatNamedArguments Args;
        int32 ArgIndex = 0;
        for (auto& Arg : URLArguments)
        {
            FString ArgStr = FString("");
            if (Arg.bUsePredefinedArgument && Arg.OnGetPredefinedArgument.IsBound())
            {
                ArgStr = Arg.OnGetPredefinedArgument.Execute(Arg.PredefinedArgument);
            }
            else
            {
                ArgStr = Arg.Argument;
            }

            Args.Add(FString::Printf(TEXT("%d"), ArgIndex), FText::FromString(ArgStr));
            ArgIndex++;
        }

        return FText::Format(FText::FromString(TargetURL), Args).ToString();
    }
};

USTRUCT(BlueprintType)
struct FFTUEDialogueModel
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly, meta = (
            Tooltip = "The Tutorial Module who owns the metadata of the FTUE.",
            DisplayThumbnail = false))
    UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "(Optional) The id to identify the FTUE. Useful when you want to get the reference to the FTUE from the source code."))
    FString FTUEId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The message to be shown on the FTUE."))
    FText Message;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Whether the FTUE message should be formatted or not."))
    bool bIsFormattedMessage = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "List of argument to be inserted on the formatted message. The order and the size of the arguments must follow the formatted message.",
        EditCondition = "bIsFormattedMessage",
        EditConditionHides))
    TArray<FTUEArgumentModel> MessageArguments;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The type of action buttons should be shown when the FTUE is shown."))
    FFTUEDialogueButtonType ButtonType = FFTUEDialogueButtonType::NO_BUTTON;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        EditCondition = "ButtonType!=FFTUEDialogueButtonType::NO_BUTTON", 
        EditConditionHides))
    FFTUEDialogueButtonModel Button1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        EditCondition = "ButtonType==FFTUEDialogueButtonType::TWO_BUTTONS", 
        EditConditionHides))
    FFTUEDialogueButtonModel Button2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Whether the FTUE should highlight a widget when it is shown."))
    bool bHighlightWidget = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        EditCondition = "bHighlightWidget", 
        EditConditionHides, 
        Tooltip = "The class type of the widget to be highlighted."))
    TSubclassOf<UCommonUserWidget> TargetWidgetClassToHighlight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        EditCondition = "bHighlightWidget", 
        EditConditionHides, 
        Tooltip = "The name of the widget to be highlighted."))
    FString TargetWidgetNameToHighlight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The vertical anchor of the FTUE relative to the screen."))
    EFTUEDialogueVerticalAnchor VerticalAnchor = EFTUEDialogueVerticalAnchor::MIDDLE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The horizontal anchor of the FTUE relative to the screen."))
    EFTUEDialogueHorizontalAnchor HorizontalAnchor = EFTUEDialogueHorizontalAnchor::MIDDLE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The position where the FTUE should be shown."))
    FVector2D Position = FVector2D(0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Whether the FTUE should lock player's accessibility to access other UIs when the FTUE is shown."))
    bool bIsInterrupting = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "Target widget where the FTUE should be shown."))
    TArray<TSubclassOf<UAccelByteWarsActivatableWidget>> TargetWidgetClasses;

    // Helper variable to sort the dialogues.
    int32 OrderPriority = 0;

    // Flag to define whether this dialogue is already shown or not.
    bool bIsAlreadyShown = false;

    // Which group owns thi dialogue (if any).
    FTUEDialogueGroup* Group = nullptr;

    // The order priority if this dialogue's group.
    int32 GroupOrderPriority = 0;

    // Helper to define whether this dialogue is the first dialogue in the group.
    bool bIsInstigator = false;

    // Helper to define whether this dialogue is the last dialogue in the group.
    bool bIsTerminator = false;

    // Event to be executed when the FTUE is shown to the screen.
    TDelegate<bool()> OnValidateDelegate;

    // Event to be executed when the FTUE is shown to the screen.
    TMulticastDelegate<void()> OnActivateDelegate;

    // Event to be executed when the FTUE is dismissed from the screen.
    TMulticastDelegate<void()> OnDeactivateDelegate;

    FVector2D GetAnchor() const
    {
        float Horizontal, Vertical;

        switch (HorizontalAnchor)
        {
        case EFTUEDialogueHorizontalAnchor::MIDDLE:
            Horizontal = 0.5f;
            break;
        case EFTUEDialogueHorizontalAnchor::LEFT:
            Horizontal = 0.0f;
            break;
        case EFTUEDialogueHorizontalAnchor::RIGHT:
            Horizontal = 1.0f;
            break;
        default:
            Horizontal = 0.5f;
            break;
        }

        switch (VerticalAnchor)
        {
        case EFTUEDialogueVerticalAnchor::MIDDLE:
            Vertical = 0.5f;
            break;
        case EFTUEDialogueVerticalAnchor::TOP:
            Vertical = 0.0f;
            break;
        case EFTUEDialogueVerticalAnchor::BOTTOM:
            Vertical = 1.0f;
            break;
        default:
            Vertical = 0.5f;
            break;
        }

        return FVector2D(Horizontal, Vertical);
    }

    FText GetFormattedMessage() const
    {
        if (!bIsFormattedMessage)
        {
            return Message;
        }

        FFormatNamedArguments Args;
        int32 ArgIndex = 0;
        for (auto& Arg : MessageArguments)
        {
            FString ArgStr = FString("");
            if (Arg.bUsePredefinedArgument && Arg.OnGetPredefinedArgument.IsBound())
            {
                ArgStr = Arg.OnGetPredefinedArgument.Execute(Arg.PredefinedArgument);
            }
            else 
            {
                ArgStr = Arg.Argument;
            }
            
            Args.Add(FString::Printf(TEXT("%d"), ArgIndex), FText::FromString(ArgStr));
            ArgIndex++;
        }

        return FText::Format(Message, Args);
    }

    static FFTUEDialogueModel* GetMetadataById(const FString& FTUEId);

    static FFTUEDialogueModel* GetMetadataById(const FString& FTUEId, TArray<FFTUEDialogueModel*>& FTUEDialogues)
    {
        const auto Result = FTUEDialogues.FindByPredicate([FTUEId](const FFTUEDialogueModel* Temp)
        {
            return Temp && Temp->FTUEId == FTUEId;
        });

        return Result ? *Result : nullptr;
    }

    static FFTUEDialogueModel* GetMetadataById(const FString& FTUEId, TArray<FFTUEDialogueModel>& FTUEDialogues)
    {
        return FTUEDialogues.FindByPredicate([FTUEId](const FFTUEDialogueModel& Temp)
        {
            return Temp.FTUEId == FTUEId;
        });
    }

    bool operator<(const FFTUEDialogueModel& Other) const
    {
        return GroupOrderPriority < Other.GroupOrderPriority || OrderPriority < Other.OrderPriority;
    }
};

USTRUCT(BlueprintType)
struct FTUEDialogueGroup
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly, meta = (
            Tooltip = "The Tutorial Module who owns the metadata of the FTUE group.",
            DisplayThumbnail = false))
    UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "The order when the FTUE group should be shown."))
    int32 OrderPriority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Tooltip = "List of FTUE dialogues in the group."))
    TArray<FFTUEDialogueModel> Dialogues;

    // Flag to define whether this dialogue group is already shown or not.
    bool bIsAlreadyShown = false;

    void SetAlreadyShown(bool bIsShown) 
    {
        for(auto& Dialogue : Dialogues) 
        {
            Dialogue.bIsAlreadyShown = bIsShown;
        }

        bIsAlreadyShown = bIsShown;
    }

    static FFTUEDialogueModel* GetMetadataById(const FString& FTUEId, TArray<FTUEDialogueGroup>& FTUEDialogueGroups)
    {
        FFTUEDialogueModel* Result = nullptr;

        for (auto& Group : FTUEDialogueGroups)
        {
            Result = Group.Dialogues.FindByPredicate([FTUEId](const FFTUEDialogueModel& Temp)
            {
                return Temp.FTUEId == FTUEId;
            });

            if (Result)
            {
                break;
            }
        }

        return Result;
    }

    bool operator<(const FTUEDialogueGroup& Other) const
    {
        return OrderPriority < Other.OrderPriority;
    }
};