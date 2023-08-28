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

		StarterOnlineSessionModulesChecker();
		DependentModuleOverride();
	}
}

void UAccelByteWarsAssetManager::EndPIE(bool bStartSimulate)
{
	Super::EndPIE(bStartSimulate);

	ResetDependentModuleOverride();
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
			// this will not be executed when starting PIE unless UnloadAssetsOfType called prior to this
			Handle->WaitUntilComplete(0.0f, false);
			
			TArray<UObject*> LoadedAssets;
			Handle->GetLoadedAssets(LoadedAssets);

			PrimaryAssetCache.Remove(AssetType);
			for (UObject* LoadedAsset : LoadedAssets)
			{
				AddAssetToCache(Cast<UAccelByteWarsDataAsset>(LoadedAsset));
			}

			TutorialModuleOverride();
			StarterOnlineSessionModulesChecker();
			if (IsRunningGame() || IsRunningDedicatedServer())
			{
				DependentModuleOverride();
			}
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
#if UE_EDITOR
	// Session Module override
	TArray<FString> SessionModuleFileNamesToBeOverriden;
#endif

	// Get values from DefaultEngine.ini
	TArray<FString> ModuleOverrides;
	GConfig->GetArray(TEXT("AccelByteTutorialModules"), TEXT("ForcedEnabledModules"), ModuleOverrides, GEngineIni);
	bool bDisableOtherModules = false;
	GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("ForceDisabledOtherModules"), bDisableOtherModules, GEngineIni);

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
#if UE_EDITOR
				if (TutorialModule->GetIsOnlineSessionActivatable())
				{
					SessionModuleFileNamesToBeOverriden.Add(Asset->GetName());
				}
#endif
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
	if (!IsRunningGame() && !IsRunningDedicatedServer())
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

			// Show additional info if one / more overriden modules is a session module
			if (SessionModuleFileNamesToBeOverriden.Num() == 1)
			{
				SubText += FString::Printf(TEXT("%s%s%s"),
					LINE_TERMINATOR,
					LINE_TERMINATOR,
					*FString("Online Session Activatable module in override detected. Make sure this module is MANUALLY active."));

				SubText += FString::Printf(TEXT("%s%s%s"),
					LINE_TERMINATOR,
					TCString<TCHAR>::Tab(1),
					*SessionModuleFileNamesToBeOverriden[0]);
			}
			else if (SessionModuleFileNamesToBeOverriden.Num() > 1)
			{
				SubText += FString::Printf(TEXT("%s%s%s"),
					LINE_TERMINATOR,
					LINE_TERMINATOR,
					*FString("Multiple Online Session Activatable module in override detected. Make sure this module is MANUALLY active."));

				SubText += FString::Printf(TEXT("%s%s%s"),
					LINE_TERMINATOR,
					TCString<TCHAR>::Tab(1),
					*FString("DA_SessionInclusiveEssentials"));
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

void UAccelByteWarsAssetManager::StarterOnlineSessionModulesChecker()
{
#if UE_EDITOR
	TArray<FString> StarterSessionModuleFileNamesActive;

	TArray<UAccelByteWarsDataAsset*> Assets = GetAllAssetsForTypeFromCache(UTutorialModuleDataAsset::TutorialModuleAssetType);
	for (UAccelByteWarsDataAsset* Asset : Assets)
	{
		if (UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(Asset))
		{
			if (TutorialModule->GetIsOnlineSessionActivatable() &&
				TutorialModule->IsActiveAndDependenciesChecked() &&
				TutorialModule->IsStarterModeActive())
			{
				StarterSessionModuleFileNamesActive.Add(Asset->GetName());
			}
		}
	}

	// show notification
	if (StarterSessionModuleFileNamesActive.Num() > 1 && !IsRunningGame() && !IsRunningDedicatedServer())
	{
		FNotificationInfo Info(FText::FromString("Starter Online Session modules"));

		// build info string
		FString SubText = "Multiple Active Starter Online Session modules found:";

		for (const FString& FileName : StarterSessionModuleFileNamesActive)
		{
			SubText += FString::Printf(TEXT("%s%s%s"),
				LINE_TERMINATOR,
				TCString<TCHAR>::Tab(1),
				*FileName);
		}

		SubText += FString::Printf(TEXT("%s%s"),
			LINE_TERMINATOR,
			*FString("Game will only use the first found module's Online Session class"));

		Info.SubText = FText::FromString(SubText);

		Info.FadeInDuration = 0.25f;
		Info.FadeOutDuration = 0.5f;
		Info.bFireAndForget = false;
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			FText::FromString("Dismiss"),
			FText::FromString("Dismiss this notification"),
			FSimpleDelegate::CreateWeakLambda(this, [this]()
			{
				OnlineSessionActiveNotificationItem->SetCompletionState(SNotificationItem::CS_None);
				OnlineSessionActiveNotificationItem->ExpireAndFadeout();
			}),
			SNotificationItem::CS_None));

		OnlineSessionActiveNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	}
#endif
}

void UAccelByteWarsAssetManager::DependentModuleOverride() const
{
	TArray<UAccelByteWarsDataAsset*> Assets = GetAllAssetsForTypeFromCache(
		UTutorialModuleDataAsset::TutorialModuleAssetType);
	for (UAccelByteWarsDataAsset* Asset : Assets)
	{
		if (UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(Asset))
		{
			if (!TutorialModule->IsActiveAndDependenciesChecked() || TutorialModule->TutorialModuleDependents.IsEmpty())
			{
				continue;
			}

			// disable module if ALL the dependants is not active
			bool bDisableModule = true;
			for (const UTutorialModuleDataAsset* Dependant : TutorialModule->TutorialModuleDependents)
			{
				if (Dependant->IsActiveAndDependenciesChecked())
				{
					bDisableModule = false;
					break;
				}
			}

			if (bDisableModule)
			{
				// no need to show notification, just disable it right away.
				TutorialModule->OverridesIsActive(false);
			}
		}
	}
}

void UAccelByteWarsAssetManager::ResetDependentModuleOverride() const
{
	TArray<UAccelByteWarsDataAsset*> Assets = GetAllAssetsForTypeFromCache(
		UTutorialModuleDataAsset::TutorialModuleAssetType);
	for (UAccelByteWarsDataAsset* Asset : Assets)
	{
		if (UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(Asset))
		{
			if (TutorialModule->IsActiveAndDependenciesChecked() || TutorialModule->TutorialModuleDependents.IsEmpty())
			{
				continue;
			}

			TutorialModule->OverridesIsActive(true);
			TutorialModule->ResetOverrides();
		}
	}
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
	const UTutorialModuleDataAsset* TutorialModule)
{
	OutIds.AddUnique(TutorialModule->CodeName);
	for (const UTutorialModuleDataAsset* Module : TutorialModule->TutorialModuleDependencies)
	{
		RecursiveGetSelfAndDependencyIds(OutIds, Module);
	}
}
