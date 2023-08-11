// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AssetManager/AccelByteWarsAssetManager.h"

#include "Framework/Notifications/NotificationManager.h"
#include "GameModes/GameModeDataAsset.h"
#include "GameModes/GameModeTypeDataAsset.h"
#include "TutorialModules/TutorialModuleDataAsset.h"
#include "Widgets/Notifications/SNotificationList.h"

UAccelByteWarsAssetManager::UAccelByteWarsAssetManager() {

}

void UAccelByteWarsAssetManager::StartInitialLoading() {
	SCOPED_BOOT_TIMING("UAccelByteWarsAssetManager::StartInitialLoading");

	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	// Populate game mode Cache on boot up
	PopulateAssetCache();
}

UAccelByteWarsAssetManager& UAccelByteWarsAssetManager::Get() {
	check(GEngine);

	if (UAccelByteWarsAssetManager* Singleton = Cast<UAccelByteWarsAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to AccelByteWarsAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UAccelByteWarsAssetManager>();
}

void UAccelByteWarsAssetManager::PopulateAssetCache()
{
	TArray<FPrimaryAssetType> AssetTypesToLoad;

	// Load tutorial module assets
	AssetTypesToLoad.Add(UTutorialModuleDataAsset::TutorialModuleAssetType);
	LoadAssetsOfType(AssetTypesToLoad);
}

#if WITH_EDITOR
void UAccelByteWarsAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);
	{
		// Do preloading here
		
		// Populate Asset Cache before playing in editor
		PopulateAssetCache();
	}
}
#endif

TArray<FGameModeData> UAccelByteWarsAssetManager::GetAllGameModes()
{
	TArray<FGameModeData> ToReturn;

	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	Get().GetPrimaryAssetIdList(UGameModeDataAsset::GameModeAssetType, PrimaryAssetIdList);

	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIdList)
	{
		const FString CodeName = UGameModeDataAsset::GetCodeNameFromAssetId(PrimaryAssetId);
		
		FGameModeData ModeData = UGameModeDataAsset::GetGameModeDataByCodeName(CodeName);
		ToReturn.Add(ModeData);
	}

	ToReturn.Sort([](const FGameModeData& LHS, const FGameModeData& RHS)
	{
		return LHS.DisplayName.ToString() < RHS.DisplayName.ToString();
	});

	return ToReturn;
}

TArray<FGameModeTypeData> UAccelByteWarsAssetManager::GetAllGameModeTypes()
{
	TArray<FGameModeTypeData> ToReturn;

	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	Get().GetPrimaryAssetIdList(UGameModeTypeDataAsset::GameModeTypeAssetType, PrimaryAssetIdList);

	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIdList)
	{
		FGameModeTypeData TypeData = UGameModeTypeDataAsset::GetGameModeTypeDataForType(PrimaryAssetId);
		ToReturn.Add(TypeData);
	}

	ToReturn.Sort([](const FGameModeTypeData& LHS, const FGameModeTypeData& RHS)
	{
		return LHS.DisplayName.ToString() < RHS.DisplayName.ToString();
	});

	return ToReturn;
}

TArray<FTutorialModuleData> UAccelByteWarsAssetManager::GetAllTutorialModules()
{
	TArray<FTutorialModuleData> ToReturn;

	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	Get().GetPrimaryAssetIdList(UTutorialModuleDataAsset::TutorialModuleAssetType, PrimaryAssetIdList);

	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIdList)
	{
		const FString CodeName = UTutorialModuleDataAsset::GetCodeNameFromAssetId(PrimaryAssetId);
		
		FTutorialModuleData ModuleData = UTutorialModuleDataAsset::GetTutorialModuleDataByCodeName(CodeName);
		
		ToReturn.Add(ModuleData);
		
	}

	ToReturn.Sort([](const FTutorialModuleData& LHS, const FTutorialModuleData& RHS)
	{
		return LHS.DisplayName.ToString() < RHS.DisplayName.ToString();
	});

	return ToReturn;
}

TArray<UAccelByteWarsDataAsset*> UAccelByteWarsAssetManager::GetAllAssetsForTypeFromCache(FPrimaryAssetType AssetType) const
{
	TArray<UAccelByteWarsDataAsset*> ToReturn;
	if (PrimaryAssetCache.Contains(AssetType))
	{
		const FPrimaryAssetCache& CacheForType = PrimaryAssetCache.FindChecked(AssetType);
		CacheForType.AssetMap.GenerateValueArray(ToReturn);
	}

	return ToReturn;
}

UAccelByteWarsDataAsset* UAccelByteWarsAssetManager::GetAssetFromCache(FPrimaryAssetId AssetId) const
{
	if (PrimaryAssetCache.Contains(AssetId.PrimaryAssetType))
	{
		const FPrimaryAssetCache& CacheForType = PrimaryAssetCache.FindChecked(AssetId.PrimaryAssetType);
		return CacheForType.AssetMap.FindRef(AssetId);
	}

	return nullptr;
}

void UAccelByteWarsAssetManager::LoadAssetsOfType(const TArray<FPrimaryAssetType>& AssetTypes)
{
	for (const FPrimaryAssetType& AssetType : AssetTypes)
	{
		TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(AssetType);
		if (Handle.IsValid())
		{
			Handle->WaitUntilComplete(0.0f, false);
			
			TArray<UObject*> LoadedAssets;
			Handle->GetLoadedAssets(LoadedAssets);

			PrimaryAssetCache.Remove(AssetType);
			for (UObject* LoadedAsset : LoadedAssets)
			{
				AddAssetToCache(Cast<UAccelByteWarsDataAsset>(LoadedAsset));
			}

			TutorialModuleOverride();
		}
	}
}

void UAccelByteWarsAssetManager::UnloadAssetsOfType(const TArray<FPrimaryAssetType>& AssetTypes)
{
	for (const FPrimaryAssetType& AssetType : AssetTypes)
	{
		PrimaryAssetCache.Remove(AssetType);
		UnloadPrimaryAssetsWithType(AssetType);
	}
}

void UAccelByteWarsAssetManager::AddAssetToCache(UAccelByteWarsDataAsset* Asset)
{
	if (Asset)
	{
		PrimaryAssetCache.FindOrAdd(Asset->GetPrimaryAssetId().PrimaryAssetType).AssetMap.Add(Asset->GetPrimaryAssetId(), Asset);
	}
}

void UAccelByteWarsAssetManager::RemoveAssetFromCache(const FPrimaryAssetId& AssetId)
{
	if (PrimaryAssetCache.Contains(AssetId.PrimaryAssetType))
	{
		PrimaryAssetCache.FindChecked(AssetId.PrimaryAssetType).AssetMap.Remove(AssetId);
	}
}

void UAccelByteWarsAssetManager::TutorialModuleOverride()
{
	// Get module override from launch parameters.
	TArray<FString> CmdModuleOverrides;
	FString CmdArgs = FCommandLine::Get();
	if (CmdArgs.Contains(TEXT("-ENABLED_MODULES="), ESearchCase::IgnoreCase))
	{
		FString CmdModuleOverridesStr;
		FParse::Value(FCommandLine::Get(), TEXT("-ENABLED_MODULES="), CmdModuleOverridesStr, false);

		// Extract the module overrides.
		CmdModuleOverridesStr = CmdModuleOverridesStr.Replace(TEXT(" "), TEXT(""));
		CmdModuleOverridesStr = CmdModuleOverridesStr.Replace(TEXT("["), TEXT(""));
		CmdModuleOverridesStr = CmdModuleOverridesStr.Replace(TEXT("]"), TEXT(""));
		CmdModuleOverridesStr.ParseIntoArray(CmdModuleOverrides, TEXT(","));

		// Module name must follow TutorialModule:MODULENAME format.
		const FString ModulePrefix = TEXT("TutorialModule:");
		for (FString& CmdModuleOverride : CmdModuleOverrides)
		{
			CmdModuleOverride.RemoveFromStart(ModulePrefix);
			CmdModuleOverride = FString::Printf(TEXT("%s%s"), *ModulePrefix, *CmdModuleOverride.ToUpper());
		}
	}

	// Get module override from DefaultEngine.ini
	TArray<FString> IniModuleOverrides;
	GConfig->GetArray(TEXT("AccelByteTutorialModules"), TEXT("ForcedEnabledModules"), IniModuleOverrides, GEngineIni);

	// Combine module override from launch parameters and DefaultEngine.ini
	TSet<FString> ModuleOverrides;
	ModuleOverrides.Append(CmdModuleOverrides);
	ModuleOverrides.Append(IniModuleOverrides);

	// Get disable other module override (priority: launch param -> DefaultEngine.ini)
	bool bDisableOtherModules = false;
	if (CmdArgs.Contains(TEXT("-DISABLE_OTHER_MODULES="), ESearchCase::IgnoreCase))
	{
		FString CmdDisableOtherModulesStr;
		FParse::Value(FCommandLine::Get(), TEXT("-DISABLE_OTHER_MODULES="), CmdDisableOtherModulesStr);

		if (CmdDisableOtherModulesStr.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase))
		{
			bDisableOtherModules = true;
		}
		else if (CmdDisableOtherModulesStr.Equals(TEXT("FALSE"), ESearchCase::IgnoreCase))
		{
			bDisableOtherModules = false;
		}
	}
	else
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("ForceDisabledOtherModules"), bDisableOtherModules, GEngineIni);
	}

	// Consolidate all modules needed to be activated
	TArray<FString> IdsToBeOverriden;
	for (const FString& ModuleOverride : ModuleOverrides)
	{
		if (UAccelByteWarsDataAsset* DataAsset = GetAssetFromCache(FPrimaryAssetId{ModuleOverride}))
		{
			if (const UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(DataAsset))
			{
				RecursiveGetSelfAndDependencyIds(IdsToBeOverriden, TutorialModule);
			}
		}
	}

	// Reset overrides and overrides the one needed to be overriden
	TArray<UAccelByteWarsDataAsset*> Assets = GetAllAssetsForTypeFromCache(UTutorialModuleDataAsset::TutorialModuleAssetType);
	for (UAccelByteWarsDataAsset* Asset : Assets)
	{
		if (UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(Asset))
		{
			if (IdsToBeOverriden.Find(TutorialModule->CodeName) != INDEX_NONE)
			{
				TutorialModule->OverridesIsActive(true);
			}
			else
			{
				if (bDisableOtherModules)
				{
					TutorialModule->OverridesIsActive(false);
				}
			}
		}
	}

#if UE_EDITOR
	if (!IsRunningGame())
	{
		// Show notification if overriden
		if (bDisableOtherModules || !IdsToBeOverriden.IsEmpty())
		{
			FNotificationInfo Info(FText::FromString("Overrides found on DefaultEngine.ini"));

			// build info string
			FString SubText;
			if (!IdsToBeOverriden.IsEmpty())
			{
				SubText += "Force enabled modules:";
				for (const FString& Id : IdsToBeOverriden)
				{
					SubText += FString::Printf(TEXT("%s%s%s"),
						LINE_TERMINATOR,
						TCString<TCHAR>::Tab(1),
						*Id);
				}
			}
			if (bDisableOtherModules)
			{
				if (!IdsToBeOverriden.IsEmpty())
				{
					SubText += FString::Printf(TEXT("%s%s"), LINE_TERMINATOR, LINE_TERMINATOR);
				}
				SubText += "All unrelated modules are disabled";
			}
			Info.SubText = FText::FromString(SubText);

			Info.HyperlinkText = FText::FromString("Edit Override");
			Info.Hyperlink = FSimpleDelegate::CreateWeakLambda(this, []()
			{
				FPlatformProcess::LaunchFileInDefaultExternalApplication(
					*FString::Printf(TEXT("%sDefaultEngine.ini"), *FPaths::SourceConfigDir()));
			});

			Info.FadeInDuration = 0.25f;
			Info.FadeOutDuration = 0.5f;
			Info.bFireAndForget = false;
			Info.ButtonDetails.Add(FNotificationButtonInfo(
				FText::FromString("Dismiss"),
				FText::FromString("Dismiss this notification"),
				FSimpleDelegate::CreateWeakLambda(this, [this]()
				{
					OverrideNotificationItem->SetCompletionState(SNotificationItem::CS_None);
					OverrideNotificationItem->ExpireAndFadeout();
				}),
				SNotificationItem::CS_None));

			OverrideNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
#endif
}

#if UE_EDITOR
void UAccelByteWarsAssetManager::PostInitialAssetScan()
{
	Super::PostInitialAssetScan();

	if (!IsRunningGame())
	{
		PopulateAssetCache();
	}
}
#endif

void UAccelByteWarsAssetManager::RecursiveGetSelfAndDependencyIds(
	TArray<FString>& OutIds,
	const UTutorialModuleDataAsset* TutorialModule) const
{
	OutIds.AddUnique(TutorialModule->CodeName);
	for (const UTutorialModuleDataAsset* Module : TutorialModule->TutorialModuleDependencies)
	{
		RecursiveGetSelfAndDependencyIds(OutIds, Module);
	}
}
