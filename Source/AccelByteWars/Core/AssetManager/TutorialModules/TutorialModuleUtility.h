// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModuleUtility.generated.h"

class UUserWidget;
class UTutorialModuleDataAsset;

USTRUCT(BlueprintType)
struct FTutorialModuleDependency
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bUseTutorialModuleDependency = false;

	/* @brief
	 * Tutorial Module that acts as part of the owner of this struct.
	 * Basically, if not null, this module will always be considered as dependency by the owner.
	 * The owner should use this dependency to perform actions with the module data, such as activating the module widget, etc.
	*/
	UPROPERTY(EditAnywhere, meta = (EditCondition = bUseTutorialModuleDependency, DisplayThumbnail = false))
	FPrimaryAssetId AssociateTutorialModule;

	/* @brief
	 * Tutorial Module dependencies that is not part of the owner of this struct.
	 * The owner should not use these dependencies to access module data.
	*/
	UPROPERTY(EditAnywhere, meta = (EditCondition = bUseTutorialModuleDependency, DisplayThumbnail = false))
	TArray<FPrimaryAssetId> TutorialModuleDependencies;
};

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* @brief
	 * Enable/disable widget based on Tutorial Module dependencies.
	 * If the dependencies are satisfied, the widget will be enabled and vice versa.
	 * @param Dependency Tutorial Module dependencies to check.
	 * @param Widget The widget to toggle its visibility and enability.
	*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Utility")
	static void ToggleWidgetBasedOnTutorialModuleDependency(const FTutorialModuleDependency& Dependency, UUserWidget* Widget);

	/* @brief
	 * Activate associated Tutorial Module widget UI.
	 * Basically, it will display the widget UI to the menu.
	 * @param Dependency Tutorial Module dependencies to check.
	 * @param Context The context of the caller of this function.
	*/
	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Utility")
	static bool ActivateTutorialModuleWidget(const FPrimaryAssetId TutorialModuleCodeName, const UObject* Context);

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