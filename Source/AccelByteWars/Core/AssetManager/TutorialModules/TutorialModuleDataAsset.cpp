// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "TutorialModuleOnlineSession.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "JsonObjectConverter.h"

DEFINE_LOG_CATEGORY(LogTutorialModuleDataAsset);

const FPrimaryAssetType	UTutorialModuleDataAsset::TutorialModuleAssetType = TEXT("TutorialModule");

TSet<FString> UTutorialModuleDataAsset::GeneratedWidgetUsedIds;
TArray<FTutorialModuleGeneratedWidget*> UTutorialModuleDataAsset::CachedGeneratedWidgets;

TSet<FString> UTutorialModuleDataAsset::FTUEDialogueUsedIds;
TArray<FFTUEDialogueModel*> UTutorialModuleDataAsset::CachedFTUEDialogues;

UTutorialModuleDataAsset::UTutorialModuleDataAsset() 
{
	AssetType = UTutorialModuleDataAsset::TutorialModuleAssetType;

	ValidateDataAssetProperties();
}

FTutorialModuleData UTutorialModuleDataAsset::GetTutorialModuleDataByCodeName(const FString& InCodeName)
{
	FPrimaryAssetId TutorialModuleAssetId = GenerateAssetIdFromCodeName(InCodeName);

	FTutorialModuleData TutorialModuleData;
	TutorialModuleData.CodeName = InCodeName;

	TutorialModuleData.DisplayName = UAccelByteWarsDataAsset::GetDisplayNameForAsset(TutorialModuleAssetId);
	TutorialModuleData.Description = UAccelByteWarsDataAsset::GetDescriptionForAsset(TutorialModuleAssetId);

	FString DefaultClassString = UAccelByteWarsDataAsset::GetMetadataForAsset<FString>(TutorialModuleAssetId, GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, DefaultUIClass));
	TSoftClassPtr<UAccelByteWarsActivatableWidget> DefaultClassPtr = TSoftClassPtr<UAccelByteWarsActivatableWidget>(DefaultClassString);
	TutorialModuleData.DefaultUIClass = UAccelByteWarsActivatableWidget::StaticClass();

	TutorialModuleData.bIsActive = UAccelByteWarsDataAsset::GetMetadataForAsset<bool>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, bIsActive));
	TutorialModuleData.bIsStarterModeActive = UAccelByteWarsDataAsset::GetMetadataForAsset<bool>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, bIsStarterModeActive));

#pragma region "Online Session"
	TutorialModuleData.bOnlineSessionModule = UAccelByteWarsDataAsset::GetMetadataForAsset<bool>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, bOnlineSessionModule));

	FString SessionClassString = TutorialModuleData.bIsStarterModeActive ?
	   UAccelByteWarsDataAsset::GetMetadataForAsset<FString>(TutorialModuleAssetId, GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, StarterOnlineSessionClass)):
	   UAccelByteWarsDataAsset::GetMetadataForAsset<FString>(TutorialModuleAssetId, GET_MEMBER_NAME_CHECKED(UTutorialModuleDataAsset, DefaultOnlineSessionClass));
	if (TSoftClassPtr<UOnlineSession> SessionClassPtr = TSoftClassPtr<UOnlineSession>(SessionClassString))
	{
		TutorialModuleData.OnlineSessionClass = SessionClassPtr.Get();
	}
#pragma endregion 

	return TutorialModuleData;
}

FPrimaryAssetId UTutorialModuleDataAsset::GenerateAssetIdFromCodeName(const FString& InCodeName)
{
	return FPrimaryAssetId(UTutorialModuleDataAsset::TutorialModuleAssetType, FName(*InCodeName));
}

FString UTutorialModuleDataAsset::GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId)
{
	check(AssetId.PrimaryAssetType == UTutorialModuleDataAsset::TutorialModuleAssetType);
	return AssetId.PrimaryAssetName.ToString();
}

TSubclassOf<UAccelByteWarsActivatableWidget> UTutorialModuleDataAsset::GetTutorialModuleUIClass()
{
	return IsStarterModeActive() ? StarterUIClass : DefaultUIClass;
}

TSubclassOf<UTutorialModuleSubsystem> UTutorialModuleDataAsset::GetTutorialModuleSubsystemClass()
{
	return IsStarterModeActive() ? StarterSubsystemClass : DefaultSubsystemClass;
}

TArray<TSubclassOf<UTutorialModuleSubsystem>> UTutorialModuleDataAsset::GetAdditionalTutorialModuleSubsystemClasses()
{
	return IsStarterModeActive() ? AdditionalStarterSubsystemClasses : AdditionalDefaultSubsystemClasses;
}

bool UTutorialModuleDataAsset::IsActiveAndDependenciesChecked() const
{
	bool bIsDependencySatisfied = true;
	for (const UTutorialModuleDataAsset* Dependency : TutorialModuleDependencies)
	{
		if (!Dependency) continue;

		if (!Dependency->IsActiveAndDependenciesChecked())
		{
			bIsDependencySatisfied = false;
			break;
		}
	}
	
	return !bIsDependencySatisfied ? false : bIsActive;
}

void UTutorialModuleDataAsset::OverridesIsActive(const bool bInIsActive)
{
	bOverriden = true;
	bIsActive = bInIsActive;
}

void UTutorialModuleDataAsset::ResetOverrides()
{
	bOverriden = false;
}

FString UTutorialModuleDataAsset::GetAttributesLocalFilePath(const FString& TutorialModuleCodeName)
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), FString::Printf(TEXT("TutorialModuleCache/%s.json"), *TutorialModuleCodeName));
}

bool UTutorialModuleDataAsset::SaveAttributesToLocal()
{
	// Delete last local file if the code name is changed.
	if (!LastCodeName.IsEmpty() && LastCodeName != CodeName)
	{
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*GetAttributesLocalFilePath(LastCodeName));
	}

	AttributesJsonObject = MakeShareable(new FJsonObject);

#pragma region "First Time User Experience (FTUE)"
	TArray<TSharedPtr<FJsonValue>> FTUEGroupStates;
	for (auto& FTUEDialogueGroup : FTUEDialogueGroups)
	{
		TSharedPtr<FJsonValue> GroupState = MakeShareable(new FJsonValueBoolean(FTUEDialogueGroup.bIsAlreadyShown));
		FTUEGroupStates.Add(GroupState);
	}
	AttributesJsonObject->SetArrayField(KEY_FTUEGROUPSTATE, FTUEGroupStates);
#pragma endregion

	// Write module local file.
	FString JsonStr;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonStr);
	if (FJsonSerializer::Serialize(AttributesJsonObject.ToSharedRef(), JsonWriter))
	{
		if (FFileHelper::SaveStringToFile(JsonStr, *GetAttributesLocalFilePath(CodeName)))
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Log, TEXT("Success to save Tutorial Module attributes to: %s"), *GetAttributesLocalFilePath(CodeName));
			return true;
		}
		else
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Warning, TEXT("Failed to save Tutorial Module attributes to: %s"), *GetAttributesLocalFilePath(CodeName));
		}
	}
	else
	{
		UE_LOG_TUTORIALMODULEDATAASSET(Warning, TEXT("Failed to save Tutorial Module attributes to: %s"), *GetAttributesLocalFilePath(CodeName));
	}

	return false;
}

bool UTutorialModuleDataAsset::LoadAttributesFromLocal()
{
	// Delete last local file if the code name is changed.
	if (!LastCodeName.IsEmpty() && LastCodeName != CodeName)
	{
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*GetAttributesLocalFilePath(LastCodeName));
	}

	// Load from local file.
	FString JsonStr;
	if (FFileHelper::LoadFileToString(JsonStr, *GetAttributesLocalFilePath(CodeName)))
	{
		AttributesJsonObject = MakeShareable(new FJsonObject);

		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonStr);
		if (FJsonSerializer::Deserialize(JsonReader, AttributesJsonObject))
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Log, TEXT("Success to load Tutorial Module attributes from: %s"), *GetAttributesLocalFilePath(CodeName));
			return true;
		}
		else
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Warning, TEXT("Failed to load Tutorial Module attributes from: %s"), *GetAttributesLocalFilePath(CodeName));
		}
	}
	else
	{
		UE_LOG_TUTORIALMODULEDATAASSET(Warning, TEXT("Failed to load Tutorial Module attributes from: %s"), *GetAttributesLocalFilePath(CodeName));
	}

	return false;
}

#pragma region "Online Session"
TSubclassOf<UTutorialModuleOnlineSession> UTutorialModuleDataAsset::GetTutorialModuleOnlineSessionClass()
{
	return IsStarterModeActive() ? StarterOnlineSessionClass : DefaultOnlineSessionClass;
}

void UTutorialModuleDataAsset::RevalidateGeneratedWidgets()
{
	ValidateGeneratedWidgets();
}
#pragma endregion

void UTutorialModuleDataAsset::ValidateDataAssetProperties()
{
	LoadAttributesFromLocal();

	// Validate Default's class properties.
	ValidateClassProperty(DefaultUIClass, LastDefaultUIClass, false);
	ValidateClassProperty(DefaultSubsystemClass, LastDefaultSubsystemClass, false);

	LastAdditionalDefaultSubsystemClasses.Empty();
	LastAdditionalDefaultSubsystemClasses.AddDefaulted(AdditionalDefaultSubsystemClasses.Num());
	for (int i = 0; i < AdditionalDefaultSubsystemClasses.Num(); ++i)
	{
		ValidateClassProperty(
			AdditionalDefaultSubsystemClasses[i],
			LastAdditionalDefaultSubsystemClasses[i],
			false);
	}

	// Validate Starter's class properties.
	ValidateClassProperty(StarterUIClass, LastStarterUIClass, true);
	ValidateClassProperty(StarterSubsystemClass, LastStarterSubsystemClass, true);

	LastAdditionalStarterSubsystemClasses.Empty();
	LastAdditionalStarterSubsystemClasses.AddDefaulted(AdditionalStarterSubsystemClasses.Num());
	for (int i = 0; i < AdditionalStarterSubsystemClasses.Num(); ++i)
	{
		ValidateClassProperty(
			AdditionalStarterSubsystemClasses[i],
			LastAdditionalStarterSubsystemClasses[i],
			true);
	}

	// Validate Default's and Starter class properties for OnlineSession module
	if (bOnlineSessionModule)
	{
		ValidateClassProperty(DefaultOnlineSessionClass, LastDefaultOnlineSessionClass, false);
		ValidateClassProperty(StarterOnlineSessionClass, LastStarterOnlineSessionClass, true);
	}

	ValidateGeneratedWidgets();
	ValidateFTUEDialogues();

	LastCodeName = CodeName;
}

bool UTutorialModuleDataAsset::ValidateClassProperty(TSubclassOf<UAccelByteWarsActivatableWidget>& UIClass, TSubclassOf<UAccelByteWarsActivatableWidget>& LastUIClass, const bool IsStarterClass)
{
	// Check if the class is used by other Tutorial Module or not.
	if (UIClass.Get() && UIClass.GetDefaultObject()->AssociateTutorialModule != nullptr
		&& UIClass.GetDefaultObject()->AssociateTutorialModule != this)
	{
#if UE_EDITOR
		ShowPopupMessage(
			FString::Printf(TEXT("UI Class %s is already being used by %s Tutorial Module"),
				*UIClass.Get()->GetName(),
				*UIClass.GetDefaultObject()->AssociateTutorialModule->GetName()));
#endif
		UIClass = nullptr;
	}

	// Reset the last class first to makes sure the references are clear.
	if (LastUIClass.Get() && LastUIClass.GetDefaultObject()
		&& LastUIClass.GetDefaultObject()->AssociateTutorialModule == this)
	{
		LastUIClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	// Update the new class to points to this Tutorial Module.
	if (UIClass.Get() && UIClass.GetDefaultObject())
	{
		UIClass.GetDefaultObject()->AssociateTutorialModule =
			((IsStarterClass && IsStarterModeActive()) || (!IsStarterClass && !IsStarterModeActive())) ? this : nullptr;
	}

	// Cache the class reference.
	LastUIClass = UIClass;

	return UIClass != nullptr;
}

bool UTutorialModuleDataAsset::ValidateClassProperty(TSubclassOf<UTutorialModuleSubsystem>& SubsystemClass, TSubclassOf<UTutorialModuleSubsystem>& LastSubsystemClass, const bool IsStarterClass)
{
	// Check if the class is used by other Tutorial Module or not.
	if (SubsystemClass.Get() && SubsystemClass.GetDefaultObject()->AssociateTutorialModule != nullptr
		&& SubsystemClass.GetDefaultObject()->AssociateTutorialModule != this)
	{
#if UE_EDITOR
		ShowPopupMessage(
			FString::Printf(TEXT("Subsystem Class %s is already being used by %s Tutorial Module"),
				*SubsystemClass.Get()->GetName(),
				*SubsystemClass.GetDefaultObject()->AssociateTutorialModule->GetName()));
#endif
		SubsystemClass = nullptr;
	}

	// Reset the last class first to makes sure the references are clear.
	if (LastSubsystemClass.Get() && LastSubsystemClass.GetDefaultObject()
		&& LastSubsystemClass.GetDefaultObject()->AssociateTutorialModule == this)
	{
		LastSubsystemClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	// Update the new class to points to this Tutorial Module
	if (SubsystemClass.Get() && SubsystemClass.GetDefaultObject())
	{
		SubsystemClass.GetDefaultObject()->AssociateTutorialModule = ((IsStarterClass && IsStarterModeActive()) || (!IsStarterClass && !IsStarterModeActive())) ? this : nullptr;
	}

	// Cache the class reference.
	LastSubsystemClass = SubsystemClass;

	return SubsystemClass != nullptr;
}

bool UTutorialModuleDataAsset::ValidateClassProperty(TSubclassOf<UTutorialModuleOnlineSession>& OnlineSessionClass, TSubclassOf<UTutorialModuleOnlineSession>&LastOnlineSessionClass, const bool IsStarterClass)
{
	// Check if the class is used by other Tutorial Module or not.
	if (OnlineSessionClass.Get() && OnlineSessionClass.GetDefaultObject()->AssociateTutorialModule != nullptr
	   && OnlineSessionClass.GetDefaultObject()->AssociateTutorialModule != this)
	{
#if UE_EDITOR
		ShowPopupMessage(
		   FString::Printf(TEXT("Subsystem Class %s is already being used by %s Tutorial Module"),
			  *OnlineSessionClass.Get()->GetName(),
			  *OnlineSessionClass.GetDefaultObject()->AssociateTutorialModule->GetName()));
#endif
		OnlineSessionClass = nullptr;
	}

	// Reset the last class first to makes sure the references are clear.
	if (LastOnlineSessionClass.Get() && LastOnlineSessionClass.GetDefaultObject()
	   && LastOnlineSessionClass.GetDefaultObject()->AssociateTutorialModule == this)
	{
		LastOnlineSessionClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	// Update the new class to points to this Tutorial Module
	if (OnlineSessionClass.Get() && OnlineSessionClass.GetDefaultObject())
	{
		OnlineSessionClass.GetDefaultObject()->AssociateTutorialModule =
		   ((IsStarterClass && IsStarterModeActive()) || (!IsStarterClass && !IsStarterModeActive())) ? this : nullptr;
	}

	// Cache the class reference.
	LastOnlineSessionClass = OnlineSessionClass;

	return OnlineSessionClass != nullptr;
}

void UTutorialModuleDataAsset::CleanUpDataAssetProperties()
{
	TutorialModuleDependencies.Empty();

	for (FTutorialModuleGeneratedWidget& GeneratedWidget : GeneratedWidgets)
	{
		for (TSubclassOf<UAccelByteWarsActivatableWidget>& TargetWidgetClass : GeneratedWidget.TargetWidgetClasses)
		{
			if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
			{
				continue;
			}

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget* Temp)
			{
				return Temp->OwnerTutorialModule == this;
			});
		}
	}
	GeneratedWidgets.Empty();

	if (DefaultUIClass.Get())
	{
		DefaultUIClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	if (DefaultSubsystemClass.Get())
	{
		DefaultSubsystemClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	for (const TSubclassOf<UTutorialModuleSubsystem>& Class : AdditionalDefaultSubsystemClasses)
	{
		Class.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	if (StarterUIClass.Get())
	{
		StarterUIClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	if (StarterSubsystemClass.Get())
	{
		StarterSubsystemClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	for (const TSubclassOf<UTutorialModuleSubsystem>& Class : AdditionalStarterSubsystemClasses)
	{
		Class.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}
}

#pragma region "Generated Widgets"
void UTutorialModuleDataAsset::ValidateGeneratedWidgets()
{
	// Clean up last generated widgets metadata to avoid duplication.
	for (FTutorialModuleGeneratedWidget& LastGeneratedWidget : LastGeneratedWidgets)
	{
		UTutorialModuleDataAsset::GeneratedWidgetUsedIds.Remove(LastGeneratedWidget.WidgetId);

		LastGeneratedWidget.DefaultTutorialModuleWidgetClass = nullptr;
		LastGeneratedWidget.StarterTutorialModuleWidgetClass = nullptr;

		LastGeneratedWidget.OtherTutorialModule = nullptr;

		for (TSubclassOf<UAccelByteWarsActivatableWidget>& TargetWidgetClass : LastGeneratedWidget.TargetWidgetClasses)
		{
			if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
			{
				continue;
			}

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget* Temp)
			{
				return !Temp || !Temp->OwnerTutorialModule || Temp->OwnerTutorialModule == this;
			});
		}
	}
	for (FTutorialModuleGeneratedWidget& GeneratedWidget : GeneratedWidgets)
	{
		for (TSubclassOf<UAccelByteWarsActivatableWidget>& TargetWidgetClass : GeneratedWidget.TargetWidgetClasses)
		{
			if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
			{
				continue;
			}

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget* Temp)
			{
				return !Temp || !Temp->OwnerTutorialModule || Temp->OwnerTutorialModule == this;
			});
		}
	}

	// Assign fresh generated widget to the target widget class.
	CachedGeneratedWidgets.RemoveAll([](const FTutorialModuleGeneratedWidget* Temp)
	{
		return !Temp;
	});
	for (FTutorialModuleGeneratedWidget& GeneratedWidget : GeneratedWidgets)
	{
		// Clean up unnecessary references.
		if (GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET_ENTRY_BUTTON &&
			GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::GENERIC_WIDGET)
		{
			GeneratedWidget.GenericWidgetClass = nullptr;
		}
		if (GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON &&
			GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET)
		{
			GeneratedWidget.OtherTutorialModule = nullptr;
		}
		if ((GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_ENTRY_BUTTON &&
			GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::TUTORIAL_MODULE_WIDGET &&
			GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_ENTRY_BUTTON &&
			GeneratedWidget.WidgetType != ETutorialModuleGeneratedWidgetType::OTHER_TUTORIAL_MODULE_WIDGET) ||
			GeneratedWidget.TutorialModuleWidgetClassType != ETutorialModuleWidgetClassType::ASSOCIATE_WIDGET_CLASS)
		{
			GeneratedWidget.DefaultTutorialModuleWidgetClass = nullptr;
			GeneratedWidget.StarterTutorialModuleWidgetClass = nullptr;
		}

		// Assign the owner of the generated widget metadata to this Tutorial Module.
		GeneratedWidget.OwnerTutorialModule = this;

		// Check if the widget id is already used.
		if (UTutorialModuleDataAsset::GeneratedWidgetUsedIds.Contains(GeneratedWidget.WidgetId))
		{
#if UE_EDITOR
			ShowPopupMessage(FString::Printf(TEXT("%s widget id is already used. Widget id must be unique."), *GeneratedWidget.WidgetId));
#endif
			GeneratedWidget.WidgetId = TEXT("");
		}
		else if (!GeneratedWidget.WidgetId.IsEmpty())
		{
			UTutorialModuleDataAsset::GeneratedWidgetUsedIds.Add(GeneratedWidget.WidgetId);
		}

		/**
		 * flag to disable widget if the current OnlineSession is not the complete OnlineSession nor is it its own OnlineSession
		 * True if not an Online Session Tutorial Module
		 */
		bool bShouldGenerateBasedOnUsedOnlineSession = true;
		if (GetIsOnlineSessionActivatable())
		{
			UAccelByteWarsAssetManager& AssetManager = UAccelByteWarsAssetManager::Get();
			TSubclassOf<UOnlineSession> PreferredOnlineSessionClass = AssetManager.GetPreferredOnlineSessionClassFromDataAsset();
			bShouldGenerateBasedOnUsedOnlineSession =
				PreferredOnlineSessionClass == AssetManager.GetCompleteOnlineSessionClassFromDataAsset() ||
				PreferredOnlineSessionClass == GetTutorialModuleOnlineSessionClass();
		}

		// Assign the generated widget to the target widget class.
		if (IsActiveAndDependenciesChecked() && bShouldGenerateBasedOnUsedOnlineSession)
		{
			for (TSubclassOf<UAccelByteWarsActivatableWidget>& TargetWidgetClass : GeneratedWidget.TargetWidgetClasses)
			{
				if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
				{
					continue;
				}
				TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.Add(&GeneratedWidget);
				CachedGeneratedWidgets.Add(&GeneratedWidget);
			}
		}
	}

	LastGeneratedWidgets = GeneratedWidgets;
}
#pragma endregion

#pragma region "First Time User Experience (FTUE)"
bool UTutorialModuleDataAsset::HasFTUE()
{
	// Set the data asset value as the initial value.
	bool bResult = bHasFTUE;

	const FString OverrideKeyword = TEXT("ForceEnableFTUE");
	const FString CmdKeyword = FString::Printf(TEXT("-%s="), *OverrideKeyword);

	// Check for launch param override.
	const FString CmdArgs = FCommandLine::Get();
	if (CmdArgs.Contains(*CmdKeyword, ESearchCase::IgnoreCase))
	{
		FString CmdIsAlwaysShowStr;
		FParse::Value(*CmdArgs, *CmdKeyword, CmdIsAlwaysShowStr);

		const bool bTemp = bResult;
		if (CmdIsAlwaysShowStr.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase))
		{
			bResult = true;
		}
		else if (CmdIsAlwaysShowStr.Equals(TEXT("FALSE"), ESearchCase::IgnoreCase))
		{
			bResult = false;
		}

		// Show log only if the config is overridden.
		if (bResult != bTemp)
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Log,
				TEXT("Launch param overrides Tutorial Module %s's enable FTUE config to %s."),
				*CodeName, 
				bResult ? TEXT("TRUE") : TEXT("FALSE"));
		}
	}
	// Check for DefaultEngine.ini override.
	else
	{
		const bool bTemp = bResult;
		bResult = GConfig->GetBoolOrDefault(TEXT("AccelByteTutorialModules"), *OverrideKeyword, bResult, GEngineIni);

		// Show log only if the config is overridden.
		if (bResult != bTemp)
		{
			UE_LOG_TUTORIALMODULEDATAASSET(Log,
				TEXT("DefaultEngine.ini overrides Tutorial Module %s's enable FTUE config to %s."),
				*CodeName,
				bResult ? TEXT("TRUE") : TEXT("FALSE"));
		}
	}

	return bResult;
}

void UTutorialModuleDataAsset::ValidateFTUEDialogues()
{
	// Clean up last FTUE dialogues metadata to avoid duplication.
	for (auto& LastFTUEDialogueGroup : LastFTUEDialogueGroups)
	{
		// Clean up each dialogues for each dialogue groups.
		for (auto& LastFTUEDialogue : LastFTUEDialogueGroup.Dialogues) 
		{
			UTutorialModuleDataAsset::FTUEDialogueUsedIds.Remove(LastFTUEDialogue.FTUEId);

			for (auto& TargetWidgetClass : LastFTUEDialogue.TargetWidgetClasses)
			{
				if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
				{
					continue;
				}

				TargetWidgetClass.GetDefaultObject()->FTUEDialogues.RemoveAll([this](const FFTUEDialogueModel* Temp)
				{
					return !Temp || !Temp->OwnerTutorialModule || Temp->OwnerTutorialModule == this;
				});
			}
		}
	}
	for (auto& FTUEDialogueGroup : FTUEDialogueGroups)
	{
		// Clean up each dialogues for each dialogue groups.
		for (auto& FTUEDialogue : FTUEDialogueGroup.Dialogues)
		{
			for (auto& TargetWidgetClass : FTUEDialogue.TargetWidgetClasses)
			{
				if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
				{
					continue;
				}

				TargetWidgetClass.GetDefaultObject()->FTUEDialogues.RemoveAll([this](const FFTUEDialogueModel* Temp)
				{
					return !Temp || !Temp->OwnerTutorialModule || Temp->OwnerTutorialModule == this;
				});
			}
		}
	}

	// Get FTUE states from local file to set whether it is already shown before or not.
	TArray<TSharedPtr<FJsonValue>> FTUEGroupStates;
	if (GetLocalAttributes())
	{
		FTUEGroupStates = GetLocalAttributes()->GetArrayField(KEY_FTUEGROUPSTATE);
	}

	// Refresh FTUE dialogues metadata.
	CachedFTUEDialogues.RemoveAll([](const FFTUEDialogueModel* Temp)
	{
		return !Temp;
	});
	int32 GroupIndex = INDEX_NONE;
	for (auto& FTUEDialogueGroup : FTUEDialogueGroups)
	{
		GroupIndex++;
		FTUEDialogueGroup.OwnerTutorialModule = this;

		// Set up each dialogues for each dialogue groups.
		int32 DialogueIndex = INDEX_NONE;
		for (auto& FTUEDialogue : FTUEDialogueGroup.Dialogues)
		{
			DialogueIndex++;
			FTUEDialogue.OwnerTutorialModule = this;

			// Set dialogue order priority.
			FTUEDialogue.Group = &FTUEDialogueGroup;
			FTUEDialogue.GroupOrderPriority = FTUEDialogueGroup.OrderPriority;
			FTUEDialogue.OrderPriority = DialogueIndex;

			// Set dialogue statuses.
			FTUEDialogue.bIsInstigator = (DialogueIndex == 0);
			FTUEDialogue.bIsTerminator = (DialogueIndex >= FTUEDialogueGroup.Dialogues.Num() - 1);

			// Reset button metadata if not used.
			switch (FTUEDialogue.ButtonType)
			{
			case FFTUEDialogueButtonType::NO_BUTTON:
				FTUEDialogue.Button1.Reset();
			case FFTUEDialogueButtonType::ONE_BUTTON:
				FTUEDialogue.Button2.Reset();
				break;
			}

			// Reset highlighted widget metadata if not used.
			if (!FTUEDialogue.bHighlightWidget)
			{
				FTUEDialogue.TargetWidgetClassToHighlight = nullptr;
				FTUEDialogue.TargetWidgetNameToHighlight = FString("");
			}

			// Check if the FTUE id is already used.
			if (UTutorialModuleDataAsset::FTUEDialogueUsedIds.Contains(FTUEDialogue.FTUEId))
			{
#if UE_EDITOR
				ShowPopupMessage(FString::Printf(TEXT("%s FTUE id is already used. FTUE id must be unique."), *FTUEDialogue.FTUEId));
#endif
				FTUEDialogue.FTUEId = TEXT("");
			}
			else if (!FTUEDialogue.FTUEId.IsEmpty())
			{
				UTutorialModuleDataAsset::FTUEDialogueUsedIds.Add(FTUEDialogue.FTUEId);
			}

			// Assign FTUE dialogue metadata to the target widget class.
			if (IsActiveAndDependenciesChecked() && HasFTUE())
			{
				for (auto& TargetWidgetClass : FTUEDialogue.TargetWidgetClasses)
				{
					if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
					{
						continue;
					}
					TargetWidgetClass.GetDefaultObject()->FTUEDialogues.Add(&FTUEDialogue);
					CachedFTUEDialogues.Add(&FTUEDialogue);
				}
			}
		}

		// Set dialogue group state.
		FTUEDialogueGroup.SetAlreadyShown(
			FTUEGroupStates.IsValidIndex(GroupIndex) && FTUEGroupStates[GroupIndex] ? 
			FTUEGroupStates[GroupIndex]->AsBool() : false);
	}

	// Save dialogues cache for clean-up later.
	LastFTUEDialogueGroups = FTUEDialogueGroups;
}
#pragma endregion

void UTutorialModuleDataAsset::PostLoad()
{
	Super::PostLoad();

	ValidateDataAssetProperties();
}

#if WITH_EDITOR
void UTutorialModuleDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ValidateDataAssetProperties();
}

void UTutorialModuleDataAsset::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	CodeName = TEXT("");
	ValidateDataAssetProperties();
}

void UTutorialModuleDataAsset::FinishDestroy()
{
	CleanUpDataAssetProperties();

	Super::FinishDestroy();
}

void UTutorialModuleDataAsset::ShowPopupMessage(const FString& Message) const
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.Image = FCoreStyle::Get().GetBrush("MessageLog.Error");
	Info.ExpireDuration = 10.0f;
	Info.FadeInDuration = 0.25f;
	Info.FadeOutDuration = 0.5f;

	FSlateNotificationManager::Get().AddNotification(Info);
}
#endif