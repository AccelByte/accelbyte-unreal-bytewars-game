// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModuleUtility.generated.h"

class UTutorialModuleDataAsset;
class UAccelByteWarsActivatableWidget;

UENUM(BlueprintType)
enum class ETutorialModuleWidgetType : uint8
{
	TUTORIAL_MODULE_ENTRY_BUTTON UMETA(DisplayName = "Tutorial Module Entry Button"),
	TUTORIAL_MODULE_DEFAULT_UI UMETA(DisplayName = "Tutorial Module Default UI"),
	OTHER_UI_ENTRY_BUTTON UMETA(DisplayName = "Other UI Entry Button"),
	OTHER_UI UMETA(DisplayName = "Other UI")
};

USTRUCT(BlueprintType)
struct FTutorialModuleWidgetConnection
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsTargetUISelf = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "!bIsTargetUISelf", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> TargetUIClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayThumbnail = false, EditCondition = "bIsTargetUISelf", EditConditionHides))
	UTutorialModuleDataAsset* SourceTutorialModule;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ETutorialModuleWidgetType WidgetType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "WidgetType==ETutorialModuleWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON||WidgetType==ETutorialModuleWidgetType::OTHER_UI_ENTRY_BUTTON", EditConditionHides))
	FText EntryButtonText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "WidgetType==ETutorialModuleWidgetType::OTHER_UI_ENTRY_BUTTON||WidgetType==ETutorialModuleWidgetType::OTHER_UI", EditConditionHides))
	TSubclassOf<UAccelByteWarsActivatableWidget> OtherUIClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 TargetWidgetContainerIndex = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "!bIsTargetUISelf", EditConditionHides))
	int32 PriorityOrder = 0;
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
	static bool ActivateTutorialModuleWidget(const UTutorialModuleDataAsset* TutorialModule, const UObject* Context);

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