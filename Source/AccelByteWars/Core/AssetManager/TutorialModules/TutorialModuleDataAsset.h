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
class UTutorialModuleSubsystem;

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

	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#endif

	static FTutorialModuleData GetTutorialModuleDataByCodeName(const FString& InCodeName);
	static FPrimaryAssetId GenerateAssetIdFromCodeName(const FString& InCodeName);
	static FString GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId);

	UFUNCTION(BlueprintPure)
	TSubclassOf<UAccelByteWarsActivatableWidget> GetTutorialModuleUIClass();
	
	UFUNCTION(BlueprintPure)
	TSubclassOf<UTutorialModuleSubsystem> GetTutorialModuleSubsystemClass();
	
	bool IsActiveAndDependenciesChecked();
	bool IsStarterModeActive() { return bIsStarterModeActive; }

	void OverridesIsActive(const bool bInIsActive);
	void ResetOverrides();

	// Alias to set for this mode (needs to be unique)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module")
	FString CodeName;

	static const FPrimaryAssetType TutorialModuleAssetType;

private:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module")
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultUIClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module")
	TSubclassOf<UTutorialModuleSubsystem> DefaultSubsystemClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter")
	TSubclassOf<UAccelByteWarsActivatableWidget> StarterUIClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter")
	TSubclassOf<UTutorialModuleSubsystem> StarterSubsystemClass;

	UPROPERTY()
	bool bOverriden = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bOverriden", HideEditConditionToggle), Category = "Tutorial Module")
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, Category = "Tutorial Module Starter")
	bool bIsStarterModeActive = false;

public:
#pragma region "Tutorial Module Dependencies"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Dependencies", meta = (DisplayThumbnail = false, ShowOnlyInnerProperties))
	TArray<UTutorialModuleDataAsset*> TutorialModuleDependencies;
#pragma endregion

#pragma region "Tutorial Module Widgets"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Widgets", meta = (ShowOnlyInnerProperties))
	TArray<FTutorialModuleWidgetConnection> OtherTutorialModuleWidgetsToThisModuleWidgetConnections;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Widgets", meta = (ShowOnlyInnerProperties))
	TArray<FTutorialModuleWidgetConnection> ThisTutorialModuleWidgetToNonTutorialModuleWidgetsConnections;
#pragma endregion

private:
	void UpdateDataAssetProperties();
	void ValidateDataAssetProperties();

	bool ValidateClassProperty(TSubclassOf<UAccelByteWarsActivatableWidget>& UIClass, TSubclassOf<UAccelByteWarsActivatableWidget>& LastUIClass, const bool IsStarterClass);
	bool ValidateClassProperty(TSubclassOf<UTutorialModuleSubsystem>& SubsystemClass, TSubclassOf<UTutorialModuleSubsystem>& LastSubsystemClass, const bool IsStarterClass);

#if WITH_EDITOR
	void ShowPopupMessage(const FString& Message) const;
#endif

	TSubclassOf<UAccelByteWarsActivatableWidget> LastDefaultUIClass;
	TSubclassOf<UTutorialModuleSubsystem> LastDefaultSubsystemClass;

	TSubclassOf<UAccelByteWarsActivatableWidget> LastStarterUIClass;
	TSubclassOf<UTutorialModuleSubsystem> LastStarterSubsystemClass;

	TArray<FTutorialModuleWidgetConnection> LastThisTutorialModuleWidgetToNonModuleWidgetsConnections;
};
