// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/AssetManager/AssetManagementSubsystem.h"
#include "Core/AssetManager/AccelByteWarsDataAsset.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#include "Core/UI/AccelByteWarsBaseUI.h"

bool UTutorialModuleUtility::ActivateTutorialModuleWidget(UTutorialModuleDataAsset* TutorialModule, const UObject* Context)
{
	if (!Context)
	{
		return false;
	}

	// Push default UI class to the menu stack.
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(Context->GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		return false;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget)
	{
		return false;
	}

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, TutorialModule->GetTutorialModuleUIClass());

	return true;
}

UTutorialModuleDataAsset* UTutorialModuleUtility::GetTutorialModuleDataAsset(const FPrimaryAssetId TutorialModuleCodeName, const UObject* Context, const bool bEnsureIsActive)
{
	if (!TutorialModuleCodeName.IsValid() || !Context)
	{
		return nullptr;
	}

	const UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(Context->GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	UAssetManagementSubsystem* AssetManagement = GameInstance->GetSubsystem<UAssetManagementSubsystem>();
	if (!AssetManagement)
	{
		return nullptr;
	}

	UAccelByteWarsDataAsset* DataAsset = AssetManagement->GetByteWarsAssetManager()->GetAssetFromCache(TutorialModuleCodeName);
	if (!DataAsset)
	{
		return nullptr;
	}

	UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(DataAsset);
	if (bEnsureIsActive && TutorialModule && !TutorialModule->IsActiveAndDependenciesChecked())
	{
		return nullptr;
	}

	return Cast<UTutorialModuleDataAsset>(DataAsset);
}

bool UTutorialModuleUtility::IsTutorialModuleActive(const FPrimaryAssetId TutorialModuleCodeName, const UObject* Context)
{
	if (!TutorialModuleCodeName.IsValid() || !Context) 
	{
		return false;
	}

	UTutorialModuleDataAsset* TutorialModule = GetTutorialModuleDataAsset(TutorialModuleCodeName, Context);
	return (TutorialModule && TutorialModule->IsActiveAndDependenciesChecked());
}

FTutorialModuleGeneratedWidget* FTutorialModuleGeneratedWidget::GetMetadataById(const FString& WidgetId)
{
	return *UTutorialModuleDataAsset::GetCachedGeneratedWidgets().
		FindByPredicate([WidgetId](const FTutorialModuleGeneratedWidget* Temp)
		{
			return Temp->WidgetId == WidgetId;
		}
	);
}
