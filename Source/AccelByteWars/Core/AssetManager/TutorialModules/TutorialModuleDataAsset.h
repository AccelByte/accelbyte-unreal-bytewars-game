// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/AssetManager/AccelByteWarsAssetModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "Core/UI/Components/Prompt/GUICheat/GUICheatModels.h"
#include "Core/UI/Components/WidgetValidator/WidgetValidatorModels.h"
#include "TutorialModuleDataAsset.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogTutorialModuleDataAsset, Log, All);
#define UE_LOG_TUTORIALMODULEDATAASSET(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogTutorialModuleDataAsset, Verbosity, Format, ##__VA_ARGS__) \
}

// Keys to access attributes saved in local file.
#define KEY_FTUEGROUPSTATE "FTUEDialogueGroupStates"

class UTutorialModuleOnlineSession;
class UAccelByteWarsActivatableWidget;
class UTutorialModuleSubsystem;

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleDataAsset : public UAccelByteWarsDataAsset
{
	GENERATED_BODY()

public:
	UTutorialModuleDataAsset();

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

	bool bIsOverridenByDependents = false;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual void FinishDestroy() override;
#endif

	static FTutorialModuleData GetTutorialModuleDataByCodeName(const FString& InCodeName);
	static FPrimaryAssetId GenerateAssetIdFromCodeName(const FString& InCodeName);
	static FString GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId);

	UFUNCTION(BlueprintPure)
	TSubclassOf<UAccelByteWarsActivatableWidget> GetTutorialModuleUIClass() const;
	
	UFUNCTION(BlueprintPure)
	TSubclassOf<UTutorialModuleSubsystem> GetTutorialModuleSubsystemClass() const;

	UFUNCTION(BlueprintPure)
	TArray<TSubclassOf<UTutorialModuleSubsystem>> GetAdditionalTutorialModuleSubsystemClasses() const;
	
	bool IsActiveAndDependenciesChecked() const;
	bool IsStarterModeActive() const { return bIsStarterModeActive; }

	void OverridesIsActive(const bool bInIsActive);
	void ResetOverrides();

	// Alias to set for this mode (needs to be unique)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module")
	FString CodeName;

	static const FPrimaryAssetType TutorialModuleAssetType;

	// Get the relative path where the attributes is saved in a local file.
	FString GetAttributesLocalFilePath(const FString& TutorialModuleCodeName);

	// Save attributes to local file as JSON literals.
	bool SaveAttributesToLocal();

	// Load attributes from local JSON file.
	bool LoadAttributesFromLocal();

	// Get attributes loaded from local file as JSON object.
	TSharedPtr<FJsonObject> GetLocalAttributes() { return AttributesJsonObject; }

#pragma region "Online Session"
	UFUNCTION(BlueprintPure)
	bool GetIsOnlineSessionActivatable() const { return bOnlineSessionModule; }

	UFUNCTION(BlueprintPure)
	TSubclassOf<UTutorialModuleOnlineSession> GetTutorialModuleOnlineSessionClass();

	void RevalidateGeneratedWidgets();
#pragma endregion 

private:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module")
	TSubclassOf<UAccelByteWarsActivatableWidget> DefaultUIClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module")
	TSubclassOf<UTutorialModuleSubsystem> DefaultSubsystemClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module")
	TArray<TSubclassOf<UTutorialModuleSubsystem>> AdditionalDefaultSubsystemClasses;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter")
	TSubclassOf<UAccelByteWarsActivatableWidget> StarterUIClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter")
	TSubclassOf<UTutorialModuleSubsystem> StarterSubsystemClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter")
	TArray<TSubclassOf<UTutorialModuleSubsystem>> AdditionalStarterSubsystemClasses;

	UPROPERTY()
	bool bOverriden = false;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, meta = (EditCondition = "!bOverriden", HideEditConditionToggle), Category = "Tutorial Module")
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Tutorial Module Starter", meta = (EditCondition = "bIsActive"))
	bool bIsStarterModeActive = false;

	// Helper to track whether the Tutorial Module code name is changed.
	FString LastCodeName;

	// Helper to store attributes loaded from local file.
	TSharedPtr<FJsonObject> AttributesJsonObject;

#pragma region "Online Session"
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Online Session Module")
	bool bOnlineSessionModule = false;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Online Session Module", meta = (EditCondition = "bOnlineSessionModule", EditConditionHides = "true"))
	TSubclassOf<UTutorialModuleOnlineSession> DefaultOnlineSessionClass;

	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Category = "Online Session Module", meta = (EditCondition = "bOnlineSessionModule", EditConditionHides = "true"))
	TSubclassOf<UTutorialModuleOnlineSession> StarterOnlineSessionClass;
#pragma endregion

public:
#pragma region "Generated Widgets"
	static TArray<FTutorialModuleGeneratedWidget*> GetCachedGeneratedWidgets()
	{
		return CachedGeneratedWidgets;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Dependencies", meta = (Tooltip = "Other Tutorial Modules that is required by this Tutorial Module", DisplayThumbnail = false, ShowOnlyInnerProperties))
	TArray<UTutorialModuleDataAsset*> TutorialModuleDependencies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Module Widgets", meta = (Tooltip = "Widgets that will be generated if this Tutorial Module active", ShowOnlyInnerProperties))
	TArray<FTutorialModuleGeneratedWidget> GeneratedWidgets;
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
private:
	UPROPERTY(EditAnywhere,
		Category = "First Time User Experience (FTUE)", 
		meta = (Tooltip = "Whether this Tutorial Module has FTUE or not."))
	bool bHasFTUE = false;

public:
	bool HasFTUE();

	static TArray<FFTUEDialogueModel*> GetCachedFTUEDialogues()
	{
		return CachedFTUEDialogues;
	}

	UPROPERTY(EditAnywhere,
		Category = "First Time User Experience (FTUE)",
		meta = (
			Tooltip = "FTUE dialogue group to be displayed when this Tutorial Module is active.",
			ShowOnlyInnerProperties,
			EditCondition = "bHasFTUE",
			EditConditionHides))
	TArray<FFTUEDialogueGroup> FTUEDialogueGroups;
#pragma endregion

#pragma region "Widget Validators"
private:
	void ValidateWidgetValidators();

	UPROPERTY(EditAnywhere,
		Category = "Widget Validator",
		meta = (Tooltip = "Whether this Tutorial Module enable widget validator feature or not."))
	bool bEnableWidgetValidator = false;

	TArray<FWidgetValidator> LastWidgetValidators;
	static TSet<FString> WidgetValidatorUsedIds;
	static TArray<FWidgetValidator*> CachedWidgetValidators;

public:
	UPROPERTY(EditAnywhere,
		Category = "Widget Validator",
		meta = (
			Tooltip = "Widget validator to be executed when this Tutorial Module is active",
			ShowOnlyInnerProperties,
			EditConditionHides))
	TArray<FWidgetValidator> WidgetValidators;

	bool IsWidgetValidatorEnabled();

	static TArray<FWidgetValidator*> GetCachedWidgetValidators()
	{
		return CachedWidgetValidators;
	}
#pragma endregion

private:
	void ValidateDataAssetProperties();

	bool ValidateClassProperty(TSubclassOf<UAccelByteWarsActivatableWidget>& UIClass, TSubclassOf<UAccelByteWarsActivatableWidget>& LastUIClass, const bool IsStarterClass);
	bool ValidateClassProperty(TSubclassOf<UTutorialModuleSubsystem>& SubsystemClass, TSubclassOf<UTutorialModuleSubsystem>& LastSubsystemClass, const bool IsStarterClass);
#pragma region "Online Session"
	bool ValidateClassProperty(TSubclassOf<UTutorialModuleOnlineSession>& OnlineSessionClass, TSubclassOf<UTutorialModuleOnlineSession>&LastOnlineSessionClass, const bool IsStarterClass);
#pragma endregion 

	void CleanUpDataAssetProperties();

#pragma region "Generated Widgets"
	void ValidateGeneratedWidgets();
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
	void ValidateFTUEDialogues();
#pragma endregion

#if WITH_EDITOR
	void ShowPopupMessage(const FString& Message) const;
#endif

	TSubclassOf<UAccelByteWarsActivatableWidget> LastDefaultUIClass;
	TSubclassOf<UTutorialModuleSubsystem> LastDefaultSubsystemClass;
	TArray<TSubclassOf<UTutorialModuleSubsystem>> LastAdditionalDefaultSubsystemClasses;

	TSubclassOf<UAccelByteWarsActivatableWidget> LastStarterUIClass;
	TSubclassOf<UTutorialModuleSubsystem> LastStarterSubsystemClass;
	TArray<TSubclassOf<UTutorialModuleSubsystem>> LastAdditionalStarterSubsystemClasses;

#pragma region "Online Session"
	TSubclassOf<UTutorialModuleOnlineSession> LastDefaultOnlineSessionClass;
	TSubclassOf<UTutorialModuleOnlineSession> LastStarterOnlineSessionClass;
#pragma endregion 

#pragma region "Generated Widgets"
	TArray<FTutorialModuleGeneratedWidget> LastGeneratedWidgets;
	static TSet<FString> GeneratedWidgetUsedIds;
	static TArray<FTutorialModuleGeneratedWidget*> CachedGeneratedWidgets;
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
	TArray<FFTUEDialogueGroup> LastFTUEDialogueGroups;
	static TSet<FString> FTUEDialogueUsedIds;
	static TArray<FFTUEDialogueModel*> CachedFTUEDialogues;
#pragma endregion

#pragma region "GUI Cheat"
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GUI Cheats")
	TArray<FGUICheatEntry> GUICheatEntries;

	static TMultiMap<FString, UGUICheatWidgetEntry*> GetCachedGUICheatEntries()
	{
		return CachedGUICheatEntries;
	}

private:
	TArray<FGUICheatEntry> LastGUICheatEntries;
	static TMultiMap<FString, UGUICheatWidgetEntry*> CachedGUICheatEntries;

	void ValidateGUICheatEntries();
#pragma endregion 
};
