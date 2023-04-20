// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
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
void UTutorialModuleDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ValidateDataAssetProperties();
	UpdateDataAssetProperties();
}

void UTutorialModuleDataAsset::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);
	CodeName = TEXT("");
	ValidateDataAssetProperties();
}

void UTutorialModuleDataAsset::PostLoad()
{
	Super::PostLoad();
	UpdateDataAssetProperties();
}

void UTutorialModuleDataAsset::UpdateDataAssetProperties()
{
#pragma region "Connect Other Tutorial Module Widgets to This Tutorial Module"
	// Refresh Default UI Class.
	// This will make sure the Associated of that Default UI Class correctly points to this Tutorial Module.
	if (LastDefaultUIClass.Get() && LastDefaultUIClass.GetDefaultObject() 
		&& LastDefaultUIClass.GetDefaultObject()->AssociateTutorialModule == this)
	{
		LastDefaultUIClass.GetDefaultObject()->AssociateTutorialModule = nullptr;
	}
	if (DefaultUIClass.Get() && DefaultUIClass.GetDefaultObject())
	{
		DefaultUIClass.GetDefaultObject()->AssociateTutorialModule = this;
	}
	LastDefaultUIClass = DefaultUIClass;

	// Refresh "Other Tutorial Module Widgets to This Tutorial Module" connections.
	for (FTutorialModuleWidgetConnection& Connection : OtherTutorialModuleWidgetsToThisModuleWidgetConnections)
	{
		Connection.bIsTargetUISelf = true;
		Connection.TargetUIClass = DefaultUIClass;
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

bool UTutorialModuleDataAsset::ValidateDataAssetProperties()
{
	if (DefaultUIClass.Get()) 
	{
		if (DefaultUIClass.GetDefaultObject()->AssociateTutorialModule != nullptr && 
			DefaultUIClass.GetDefaultObject()->AssociateTutorialModule != this)
		{
			ShowPopupMessage(
				FString::Printf(TEXT("Default UI Class %s is already being used by %s Tutorial Module"),
				*DefaultUIClass.Get()->GetName(),
				*DefaultUIClass.GetDefaultObject()->AssociateTutorialModule->GetName()));
			DefaultUIClass = nullptr;	
			return false;
		}
	}

	return true;
}

void UTutorialModuleDataAsset::ShowPopupMessage(const FString& Message)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.Image = FCoreStyle::Get().GetBrush("MessageLog.Error");
	Info.ExpireDuration = 10.0f;
	Info.FadeInDuration = 0.25f;
	Info.FadeOutDuration = 0.5f;

	FSlateNotificationManager::Get().AddNotification(Info);
}
#endif