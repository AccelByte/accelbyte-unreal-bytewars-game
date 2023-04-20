// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/AssetManager/AccelByteWarsAssetModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "TutorialModuleDataAsset.generated.h"

class UAccelByteWarsActivatableWidget;

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UTutorialModuleDataAsset()
	{
		AssetType = UTutorialModuleDataAsset::TutorialModuleAssetType;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		if (CodeName.IsEmpty())
		{
			return Super::GetPrimaryAssetId();
		}

		// Use Alias for Game Mode AssetId for easy lookup
		return UTutorialModuleDataAsset::GenerateAssetIdFromCodeName(CodeName);
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual void PostLoad() override;
#endif

	static FTutorialModuleData GetTutorialModuleDataByCodeName(const FString& InCodeName);
	static FPrimaryAssetId GenerateAssetIdFromCodeName(const FString& InCodeName);
	static FString GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId);
	bool IsActiveAndDependenciesChecked();

	// The UI class that used as the entrypoint from main menu to the tutorial module
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultUIClass;

	// Alias to set for this mode (needs to be unique)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CodeName;

	static const FPrimaryAssetType TutorialModuleAssetType;

	void OverridesIsActive(const bool bInIsActive);
	void ResetOverrides();

private:
	UPROPERTY()
	bool bOverriden = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition ="!bOverriden", HideEditConditionToggle))
	bool bIsActive = true;

public:
#pragma region "Tutorial Module Dependencies"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Dependencies", meta = (DisplayThumbnail = false, ShowOnlyInnerProperties))
	TArray<UTutorialModuleDataAsset*> TutorialModuleDependencies;
#pragma endregion

#pragma region "Tutorial Module Widgets"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Widgets", meta = (ShowOnlyInnerProperties))
	TArray<FTutorialModuleWidgetConnection> OtherTutorialModuleWidgetsToThisModuleWidgetConnections;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Widgets", meta = (ShowOnlyInnerProperties))
	TArray<FTutorialModuleWidgetConnection> ThisTutorialModuleWidgetToNonTutorialModuleWidgetsConnections;
#pragma endregion
#endif

#if WITH_EDITOR
private:
	void UpdateDataAssetProperties();
	bool ValidateDataAssetProperties();
	void ShowPopupMessage(const FString& Message);
	TSubclassOf<UAccelByteWarsActivatableWidget> LastDefaultUIClass;
	TArray<FTutorialModuleWidgetConnection> LastThisTutorialModuleWidgetToNonModuleWidgetsConnections;
#endif
};
