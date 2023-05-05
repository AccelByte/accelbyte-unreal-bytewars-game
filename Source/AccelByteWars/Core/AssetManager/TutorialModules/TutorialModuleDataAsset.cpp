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

#if WITH_EDITOR
void UTutorialModuleDataAsset::PostLoad()
{
	Super::PostLoad();

	UpdateDataAssetProperties();
}

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

void UTutorialModuleDataAsset::UpdateDataAssetProperties()
{
	ValidateDataAssetProperties();

#pragma region "Connect Other Tutorial Module Widgets to This Tutorial Module"
	// Refresh "Other Tutorial Module Widgets to This Tutorial Module" connections.
	for (FTutorialModuleWidgetConnection& Connection : OtherTutorialModuleWidgetsToThisModuleWidgetConnections)
	{
		Connection.bIsTargetUISelf = true;
		Connection.TargetUIClass = GetTutorialModuleUIClass();
	}
#pragma endregion

#pragma region "Connect This Tutorial Module Widgets to Non Tutorial Module"
	// Reset Target UI Class of "This Tutorial Module Widgets to Non Tutorial Module" connections.
	// This will makes sure the DefaultObject of TargetUIClass corretly points to this Tutorial Module.
	for (FTutorialModuleWidgetConnection& LastConnection : LastThisTutorialModuleWidgetToNonModuleWidgetsConnections)
	{
		if (!LastConnection.TargetUIClass || !LastConnection.TargetUIClass.GetDefaultObject()) continue;
		LastConnection.TargetUIClass.GetDefaultObject()->DissociateTutorialModuleWidgets.RemoveAll([this](const FTutorialModuleWidgetConnection& Temp)
		{
			return Temp.SourceTutorialModule == this;
		});
	}
	for (FTutorialModuleWidgetConnection& Connection : ThisTutorialModuleWidgetToNonTutorialModuleWidgetsConnections)
	{
		if (!Connection.TargetUIClass || !Connection.TargetUIClass.GetDefaultObject()) continue;
		Connection.TargetUIClass.GetDefaultObject()->DissociateTutorialModuleWidgets.RemoveAll([this](const FTutorialModuleWidgetConnection& Temp)
		{
			return Temp.SourceTutorialModule == this;
		});
	}

	// Refresh "This Tutorial Module Widgets to Non Tutorial Module" connections.
	for (FTutorialModuleWidgetConnection& Connection : ThisTutorialModuleWidgetToNonTutorialModuleWidgetsConnections)
	{
		Connection.bIsTargetUISelf = false;
		Connection.SourceTutorialModule = this;

		if (Connection.TargetUIClass && Connection.TargetUIClass.GetDefaultObject())
		{
			Connection.TargetUIClass.GetDefaultObject()->DissociateTutorialModuleWidgets.Add(Connection);
		}
	}
	LastThisTutorialModuleWidgetToNonModuleWidgetsConnections = ThisTutorialModuleWidgetToNonTutorialModuleWidgetsConnections;
#pragma endregion
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
		ShowPopupMessage(
			FString::Printf(TEXT("UI Class %s is already being used by %s Tutorial Module"),
				*UIClass.Get()->GetName(),
				*UIClass.GetDefaultObject()->AssociateTutorialModule->GetName()));

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
		ShowPopupMessage(
			FString::Printf(TEXT("Subsystem Class %s is already being used by %s Tutorial Module"),
				*SubsystemClass.Get()->GetName(),
				*SubsystemClass.GetDefaultObject()->AssociateTutorialModule->GetName()));

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