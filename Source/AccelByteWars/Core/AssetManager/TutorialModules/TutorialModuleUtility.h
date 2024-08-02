// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/UI/AccelByteWarsWidgetModels.h"
#include "TutorialModuleUtility.generated.h"

class UUserWidget;
class UTutorialModuleDataAsset;
class UAccelByteWarsActivatableWidget;

UENUM(BlueprintType)
enum class EServicePredefinedValidator : uint8
{
	NONE = 0 UMETA(DisplayName = "None"),
	IS_CUSTOM UMETA(DisplayName = "Is Custom"),
	IS_REQUIRED_AMS_ACCOUNT UMETA(DisplayName = "Is Required AMS Account"),
	IS_VALID_CONFIG_VERSION UMETA(DisplayName = "Is Valid Config Version"),
	IS_ONLINE_SESSION UMETA(DisplayName = "In Online Session"),
	IS_LOCAL_NETWORK UMETA(DisplayName = "Is Local Network"),
	IS_P2P_NETWORK UMETA(DisplayName = "Is P2P Network"),
	IS_DS_NETWORK UMETA(DisplayName = "Is DS Network")
};

USTRUCT(BlueprintType)
struct FServicePredefinedValidator
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		Tooltip = "Validation to perform. If Custom Validation selected, you need to implement it on your own through code."))
	EServicePredefinedValidator ValidatorType = EServicePredefinedValidator::NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		Tooltip = "Whether result from the validation should be negated or not."))
	bool bNegateValidator = false;
};

UENUM(BlueprintType)
enum class EServicePredifinedArgument : uint8
{
	PLAYER_ID = 0 UMETA(DisplayName = "Player Id"),
	PLAYER_DISPLAY_NAME UMETA(DisplayName = "Player Display Name"),
	GAME_SESSION_ID UMETA(DisplayName = "Game Session Id"),
	PARTY_SESSION_ID UMETA(DisplayName = "Party Session Id"),
	DEDICATED_SERVER_ID UMETA(DisplayName = "Dedicated Server Id"),
	ENV_BASE_URL UMETA(DisplayName = "Environment Base URL"),
	GAME_NAMESPACE UMETA(DisplayName = "Game Namespace"),
	ADMIN_PORTAL_URL UMETA(DisplayName = "Admin Portal URL"),
	GAME_VERSION UMETA(DisplayName = "Game Version"),
	PUBLISHED_STORE_ID UMETA(DisplayName = "Published Store Id"),
	TIME_UTC_NOW UMETA(DisplayName = "Time UTC Now")
};

USTRUCT(BlueprintType)
struct FServiceArgumentModel
{
	GENERATED_BODY()

	UPROPERTY(
		BlueprintReadOnly,
		EditAnywhere,
		meta = (Tooltip = "Whether to use predefined argument or not."))
	bool bUsePredefinedArgument = false;

	UPROPERTY(
		BlueprintReadOnly, 
		EditAnywhere, 
		meta = (
			Tooltip = "User defined string argument.",
			EditCondition = "!bUsePredefinedArgument",
			EditConditionHides))
	FString Argument;

	UPROPERTY(
		BlueprintReadOnly, 
		EditAnywhere, meta = (
			Tooltip = "Predefined argument.",
			EditCondition = "bUsePredefinedArgument",
			EditConditionHides))
	EServicePredifinedArgument PredefinedArgument = EServicePredifinedArgument::PLAYER_ID;

	// Event to get predefined argument from online tutorial modules branch.
	inline static TDelegate<FString(const EServicePredifinedArgument /*Keyword*/)> OnGetPredefinedArgument;
};

UENUM(BlueprintType)
enum class ETutorialModuleGeneratedWidgetType : uint8
{
	TUTORIAL_MODULE_ENTRY_BUTTON UMETA(DisplayName = "Tutorial Module Entry Button"),
	TUTORIAL_MODULE_WIDGET UMETA(DisplayName = "Tutorial Module Widget"),
	TUTORIAL_MODULE_ENTRY_TABLIST UMETA(DisplayName = "Tutorial Module Entry Tab List", ToolTip = "Generate tab list entry in target widget"),
	OTHER_TUTORIAL_MODULE_ENTRY_BUTTON UMETA(DisplayName = "Other Tutorial Module Entry Button"),
	OTHER_TUTORIAL_MODULE_WIDGET UMETA(DisplayName = "Other Tutorial Module Widget"),
	OTHER_TUTORIAL_MODULE_ENTRY_TABLIST UMETA(DisplayName = "Other Tutorial Module Entry Tab List", ToolTip = "Generate tab list entry in target widget. Check if other tutorial module active"),
	GENERIC_WIDGET_ENTRY_BUTTON UMETA(DisplayName = "Generic Widget Entry Button"),
	GENERIC_WIDGET UMETA(DisplayName = "Generic Widget"),
	ACTION_BUTTON UMETA(DisplayName = "Action Button")
};

UENUM(BlueprintType)
enum class ETutorialModuleWidgetClassType : uint8
{
	DEFAULT_WIDGET_CLASS UMETA(DisplayName = "Default Widget Class"),
	ASSOCIATE_WIDGET_CLASS UMETA(DisplayName = "Associate Widget Class")
};

UENUM(BlueprintType)
enum class ETutorialModuleButtonType : uint8
{
	TEXT_BUTTON UMETA(DisplayName = "Text Button"),
	IMAGE_BUTTON UMETA(DisplayName = "Image Button")
};

USTRUCT(BlueprintType)
struct FTutorialModuleGeneratedWidget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Tooltip = "The Tutorial Module who owns the metadata of to generate this generated widget.", DisplayThumbnail = false))
	UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The type of the generated widget"))
	ETutorialModuleGeneratedWidgetType WidgetType = ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "(Optional) The id to identify the generated widget. Useful when you want to get the reference to the generated widget."))
	FString WidgetId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The other Tutorial Module that its entry or associate widgets will be opened when the entry button is clicked.", DisplayThumbnail = false, EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	UTutorialModuleDataAsset* OtherTutorialModule = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The type of Tutorial Module entry widget to be opened when the entry button is clicked", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	ETutorialModuleWidgetClassType TutorialModuleWidgetClassType = ETutorialModuleWidgetClassType::DEFAULT_WIDGET_CLASS;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The widget to be opened when the entry button is clicked or to be generated if the Tutorial Module is not in starter mode", EditCondition = "TutorialModuleWidgetClassType==ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS&&(WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST)", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultTutorialModuleWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The widget to be opened when the entry button is clicked or to be generated if the Tutorial Module is in starter mode", EditCondition = "TutorialModuleWidgetClassType==ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS&&(WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST)", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> StarterTutorialModuleWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The widget class that will be generated or will be opened when the entry button is clicked", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> GenericWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The base UI target stack where the widget class will be opened when the entry button is clicked", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON", EditConditionHides))
	EBaseUIStackType TargetStackToSpawn = EBaseUIStackType::Menu;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The text that will be displayed on the entry button", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::ACTION_BUTTON", EditConditionHides))
	ETutorialModuleButtonType ButtonType = ETutorialModuleButtonType::TEXT_BUTTON;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The text that will be displayed on the entry button", EditCondition = "(ButtonType==ETutorialModuleButtonType::TEXT_BUTTON&&(WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::ACTION_BUTTON)||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	FText ButtonText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "(Optional) Input to trigger the entry button", EditCondition = "(ButtonType==ETutorialModuleButtonType::TEXT_BUTTON&&(WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::ACTION_BUTTON)||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	FDataTableRowHandle ButtonInputActionData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The tab id used for the generated entry", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	FName TabNameId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The index of the generated entry. -1 to add to last.", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST||WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	int32 TabIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The image that will be displayed on the entry button", EditCondition = "ButtonType==ETutorialModuleButtonType::IMAGE_BUTTON&&(WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::ACTION_BUTTON)", EditConditionHides))
	FSlateBrush ButtonImage;

	FSimpleMulticastDelegate ButtonAction;
	TDelegate<bool()> ValidateButtonAction;
	FSimpleMulticastDelegate OnWidgetGenerated;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The Target Widget where the generated widget will be spawned"))
	TArray<TSubclassOf<UAccelByteWarsActivatableWidget>> TargetWidgetClasses;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The container of the Target Widget where the generated widget will be spawned"))
	int32 TargetWidgetContainerIndex = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The order of the generated widget when it is spawned on the Target Widget container", EditCondition = "WidgetType!=ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_TABLIST&&WidgetType!=ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_TABLIST", EditConditionHides))
	int32 SpawnOrder = 0;

	UUserWidget* GenerateWidgetRef = nullptr;

	static FTutorialModuleGeneratedWidget* GetMetadataById(const FString& WidgetId);

	static FTutorialModuleGeneratedWidget* GetMetadataById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget*>& GeneratedWidgets)
	{
		const auto Result = GeneratedWidgets.FindByPredicate([WidgetId](const FTutorialModuleGeneratedWidget* Temp)
		{
			return Temp && Temp->WidgetId == WidgetId;
		});

		return Result ? *Result : nullptr;
	}

	static FTutorialModuleGeneratedWidget* GetMetadataById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget>& GeneratedWidgets)
	{
		return GeneratedWidgets.FindByPredicate([WidgetId](const FTutorialModuleGeneratedWidget& Temp)
		{
			return Temp.WidgetId == WidgetId;
		});
	}

	template<typename T>
	static T* GetGeneratedWidgetById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget*>& GeneratedWidgets)
	{
		FTutorialModuleGeneratedWidget* Temp = GetMetadataById(WidgetId, GeneratedWidgets);
		return Temp ? Cast<T>(Temp->GenerateWidgetRef) : nullptr;
	}

	template<typename T>
	static T* GetGeneratedWidgetById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget>& GeneratedWidgets)
	{
		UUserWidget* Temp = GetMetadataById(WidgetId, GeneratedWidgets)->GenerateWidgetRef;
		return Temp ? Cast<T>(Temp) : nullptr;
	}

	bool operator<(const FTutorialModuleGeneratedWidget& Other) const 
	{
		return SpawnOrder < Other.SpawnOrder;
	}
};

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* @brief
	 * Activate associated Tutorial Module widget UI.
	 * Basically, it will display the widget UI to the menu.
	 * @param Dependency Tutorial Module dependencies to check.
	 * @param Context The context of the caller of this function.
	*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Utility")
	static bool ActivateTutorialModuleWidget(UTutorialModuleDataAsset* TutorialModule, const UObject* Context);

	/* @brief
	 * Get Tutorial Module data asset from its code name.
	 * @param TutorialModuleCodeName Tutorial Module's code name to check.
	 * @param Context The context of the caller of this function.
	*/
	UFUNCTION(BlueprintPure, Category = "Tutorial Module Utility")
	static UTutorialModuleDataAsset* GetTutorialModuleDataAsset(const FPrimaryAssetId TutorialModuleCodeName, const UObject* Context, const bool bEnsureIsActive = false);

	UFUNCTION(BlueprintPure, Category = "Tutorial Module Utility")
	static bool IsTutorialModuleActive(const FPrimaryAssetId TutorialModuleCodeName, const UObject* Context);
};