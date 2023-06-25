// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModuleUtility.generated.h"

DECLARE_DELEGATE(FOnTutorialModuleButtonClicked);

class UUserWidget;
class UTutorialModuleDataAsset;
class UAccelByteWarsActivatableWidget;

UENUM(BlueprintType)
enum class ETutorialModuleGeneratedWidgetType : uint8
{
	TUTORIAL_MODULE_ENTRY_BUTTON UMETA(DisplayName = "Tutorial Module Entry Button"),
	TUTORIAL_MODULE_WIDGET UMETA(DisplayName = "Tutorial Module Widget"),
	OTHER_TUTORIAL_MODULE_ENTRY_BUTTON UMETA(DisplayName = "Other Tutorial Module Entry Button"),
	OTHER_TUTORIAL_MODULE_WIDGET UMETA(DisplayName = "Other Tutorial Module Widget"),
	GENERIC_WIDGET_ENTRY_BUTTON UMETA(DisplayName = "Generic Widget Entry Button"),
	GENERIC_WIDGET UMETA(DisplayName = "Generic Widget"),
	ACTION_BUTTON UMETA(DisplayName = "Action Button")
};

USTRUCT(BlueprintType)
struct FTutorialModuleGeneratedWidget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The type of the generated widget"))
	ETutorialModuleGeneratedWidgetType WidgetType = ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "(Optional) The id to identify the generated widget. Useful when you want to get the reference to the generated widget."))
	FString WidgetId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The other Tutorial Module that its entry or associate widgets will be opened when the entry button is clicked.", DisplayThumbnail = false, EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET", EditConditionHides))
	UTutorialModuleDataAsset* OtherTutorialModule = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The index of Tutorial Module associate widgets to be opened when the entry button is clicked. Default value is -1 to open the default entry widget.", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON", EditConditionHides))
	int32 AssociateEntryWidgetIndex = -1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The widget class that will be generated or will be opened when the entry button is clicked", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> GenericWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The text that will be displayed on the entry button", EditCondition = "WidgetType==ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON||WidgetType==ETutorialModuleGeneratedWidgetType::ACTION_BUTTON", EditConditionHides))
	FText ButtonText;

	FOnTutorialModuleButtonClicked ButtonAction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The Target Widget where the generated widget will be spawned"))
	TSubclassOf<UAccelByteWarsActivatableWidget> TargetWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The container of the Target Widget where the generated widget will be spawned"))
	int32 TargetWidgetContainerIndex = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The order of the generated widget when it is spawned on the Target Widget container"))
	int32 SpawnOrder = 0;

	UUserWidget* GenerateWidgetRef = nullptr;

	static FTutorialModuleGeneratedWidget* GetMetadataById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget>& GeneratedWidgets)
	{
		return GeneratedWidgets.FindByPredicate([WidgetId](const FTutorialModuleGeneratedWidget& Temp)
		{
			return Temp.WidgetId == WidgetId;
		});
	}

	template<typename T>
	static T* GetGeneratedWidgetById(const FString& WidgetId, TArray<FTutorialModuleGeneratedWidget>& GeneratedWidgets)
	{
		UUserWidget* Temp = GetMetadataById(WidgetId, GeneratedWidgets)->GenerateWidgetRef;
		return Temp ? Cast<T>(Temp) : nullptr;
	}
};

USTRUCT(BlueprintType)
struct FTutorialModuleAssociateWidget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The default associate widget"))
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultWidgetClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The starter version of the associate widget"))
	TSubclassOf<UAccelByteWarsActivatableWidget> StarterWidgetClass;
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