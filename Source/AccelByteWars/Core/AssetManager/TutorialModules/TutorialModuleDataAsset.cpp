// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Widgets/SWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectIterator.h"
#include "UObject/PropertyPortFlags.h"
#include "Components/PanelWidget.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

const FPrimaryAssetType	UTutorialModuleDataAsset::TutorialModuleAssetType = TEXT("TutorialModule");
TSet<FString> UTutorialModuleDataAsset::GeneratedWidgetUsedIds;

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

bool UTutorialModuleDataAsset::IsActiveAndDependenciesChecked()
{
	bool bIsDependencySatisfied = true;
	for (const UTutorialModuleDataAsset* Dependency : TutorialModuleDependencies)
	{
		if (!Dependency) continue;

		if (!Dependency->bIsActive)
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

void UTutorialModuleDataAsset::UpdateDataAssetProperties()
{
	ValidateDataAssetProperties();

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

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget& Temp)
			{
				return Temp.OwnerTutorialModule == this;
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

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget& Temp)
			{
				return Temp.OwnerTutorialModule == this;
			});
		}
	}

	// Assign fresh generated widget to the target widget class.
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
			ShowPopupMessage(FString::Printf(TEXT("%s widget id is already used. Widget id must be unique."), *GeneratedWidget.WidgetId));
			GeneratedWidget.WidgetId = TEXT("");
		}
		else if (!GeneratedWidget.WidgetId.IsEmpty())
		{
			UTutorialModuleDataAsset::GeneratedWidgetUsedIds.Add(GeneratedWidget.WidgetId);
		}

		// Assign the generated widget to the target widget class.
		for (TSubclassOf<UAccelByteWarsActivatableWidget>& TargetWidgetClass : GeneratedWidget.TargetWidgetClasses)
		{
			if (!TargetWidgetClass || !TargetWidgetClass.GetDefaultObject())
			{
				continue;
			}
			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.Add(GeneratedWidget);
		}
	}

	LastGeneratedWidgets = GeneratedWidgets;
}

void UTutorialModuleDataAsset::ValidateDataAssetProperties()
{
	// Validate Default's class properties.
	ValidateClassProperty(DefaultUIClass, LastDefaultUIClass, false);
	ValidateClassProperty(DefaultSubsystemClass, LastDefaultSubsystemClass, false);

	// Validate Starter's class properties.
	ValidateClassProperty(StarterUIClass, LastStarterUIClass, true);
	ValidateClassProperty(StarterSubsystemClass, LastStarterSubsystemClass, true);
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
		SubsystemClass.GetDefaultObject()->AssociateTutorialModule =
			((IsStarterClass && IsStarterModeActive()) || (!IsStarterClass && !IsStarterModeActive())) ? this : nullptr;
	}

	// Cache the class reference.
	LastSubsystemClass = SubsystemClass;

	return SubsystemClass != nullptr;
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

			TargetWidgetClass.GetDefaultObject()->GeneratedWidgets.RemoveAll([this](const FTutorialModuleGeneratedWidget& Temp)
			{
				return Temp.OwnerTutorialModule == this;
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

	if (StarterUIClass.Get())
	{
		StarterUIClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}

	if (StarterSubsystemClass.Get())
	{
		StarterSubsystemClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}
}

void UTutorialModuleDataAsset::PostLoad()
{
	Super::PostLoad();

	UpdateDataAssetProperties();
}

#if WITH_EDITOR
void UTutorialModuleDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateDataAssetProperties();
}

void UTutorialModuleDataAsset::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	CodeName = TEXT("");
	UpdateDataAssetProperties();
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

void UTutorialModuleDataAsset::FinishDestroy()
{
	CleanUpDataAssetProperties();

	Super::FinishDestroy();
}