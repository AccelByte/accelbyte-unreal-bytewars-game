// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/AccelByteWarsAssetManager.h"

#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/OnlineSession.h"
#include "GameModes/GameModeDataAsset.h"
#include "GameModes/GameModeTypeDataAsset.h"
#include "TutorialModules/TutorialModuleDataAsset.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsAssetManager);

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

	UE_LOG_ASSET_MANAGER(Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to AccelByteWarsAssetManager!"));

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

#pragma region "Online Session"
		// Reset Online Session
		PreferredOnlineSessionClass = nullptr;

		// Re run generated widget validation so that Online Session related modules will generate its widget properly
		TArray<UAccelByteWarsDataAsset*> Assets = GetAllAssetsForTypeFromCache(UTutorialModuleDataAsset::TutorialModuleAssetType);
		for (UAccelByteWarsDataAsset* Asset : Assets)
		{
			if (UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(Asset))
			{
				TutorialModule->RevalidateGeneratedWidgets();
			}
		}
#pragma endregion

		// Populate Asset Cache before playing in editor
		PopulateAssetCache();

		StarterOnlineSessionModulesChecker();
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

#pragma region "Online Session"
TSubclassOf<UOnlineSession> UAccelByteWarsAssetManager::GetCompleteOnlineSessionClassFromDataAsset()
{
	if (!CompleteOnlineSessionClass)
	{
		TArray<FTutorialModuleData> TutorialModules = UAccelByteWarsAssetManager::GetAllTutorialModules();
		for (const FTutorialModuleData& TutorialModule : TutorialModules)
		{
			if (TutorialModule.CodeName.Equals(OnlineSessionCompleteTutorialModuleCodeName))
			{
				CompleteOnlineSessionClass = TutorialModule.OnlineSessionClass;
			}
		}
	}

	return CompleteOnlineSessionClass;
}

TSubclassOf<UOnlineSession> UAccelByteWarsAssetManager::GetPreferredOnlineSessionClassFromDataAsset()
{
	if (!PreferredOnlineSessionClass)
	{
		bool bUseCompletedOnlineSession = false;
		TArray<FTutorialModuleData> TutorialModules = GetAllTutorialModules();

		for (const FTutorialModuleData& TutorialModule : TutorialModules)
		{
			if (TutorialModule.CodeName.Equals(OnlineSessionCompleteTutorialModuleCodeName))
			{
				continue;
			}

			if (TutorialModule.bIsActive && TutorialModule.bOnlineSessionModule)
			{
				/**
				 * Online Session module behaviour:
				 *
				 * if one or more of the Online Session module has starter module activated:
				 * Use the Online Session from the first module found.
				 * If more than one, show notification that the game will only use the first one.
				 * 
				 * else if none of the Online Session module has starter mode activated:
				 * if 1 Online Session module found, use its Online Session.
				 * If more than one Online Session module active, use the complete OnlineSession.
				 */
				if (TutorialModule.bIsStarterModeActive)
				{
					if (TutorialModule.OnlineSessionClass)
					{
						UE_LOG_ASSET_MANAGER(
							Log,
							TEXT("Starter online session module detected: %s"),
							*TutorialModule.OnlineSessionClass->GetPathName())

						PreferredOnlineSessionClass = TutorialModule.OnlineSessionClass;
						bUseCompletedOnlineSession = false;
						break;
					}
				}
				else
				{
					if (bUseCompletedOnlineSession)
					{
						continue;
					}

					if (TutorialModule.OnlineSessionClass)
					{
						if (!PreferredOnlineSessionClass)
						{
							PreferredOnlineSessionClass = TutorialModule.OnlineSessionClass;
						}
						else
						{
							bUseCompletedOnlineSession = true;
							UE_LOG_ASSET_MANAGER(
								Log,
								TEXT("Detected multiple Online Session module: %s and %s | Continuing search in case there's starter module activated"),
								*PreferredOnlineSessionClass->GetPathName(),
								*TutorialModule.OnlineSessionClass->GetPathName())
						}
					}
				}
			}
		}

		if (bUseCompletedOnlineSession)
		{
			PreferredOnlineSessionClass = GetCompleteOnlineSessionClassFromDataAsset();
		}

		UE_LOG_ASSET_MANAGER(Log, TEXT("OnlineSession used: %s"), *PreferredOnlineSessionClass->GetPathName());
	}

	return PreferredOnlineSessionClass;
}
#pragma endregion 

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

		UE_LOG_ASSET_MANAGER(Log, 
			TEXT("Launch param overrides the Disable Other Tutorial Modules config to %s."), 
			bDisableOtherModules ? TEXT("TRUE") : TEXT("FALSE"));
	}
	else
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("ForceDisabledOtherModules"), bDisableOtherModules, GEngineIni);

		UE_LOG_ASSET_MANAGER(Log, 
			TEXT("DefaultEngine.ini overrides the Disable Other Tutorial Modules config to %s."), 
			bDisableOtherModules ? TEXT("TRUE") : TEXT("FALSE"));
	}

	// Consolidate all modules needed to be activated
	TArray<FString> IdsToBeOverriden;
	for (const FString& ModuleOverride : ModuleOverrides)
	{
		if (UAccelByteWarsDataAsset* DataAsset = GetAssetFromCache(FPrimaryAssetId{ModuleOverride}))
		{
			if (const UTutorialModuleDataAsset* TutorialModule = Cast<UTutorialModuleDataAsset>(DataAsset))
			{
				const bool bIsCmdOverride = CmdModuleOverrides.Contains(ModuleOverride);
				const bool bIsIniOverride = IniModuleOverrides.Contains(ModuleOverride);
				if (bIsCmdOverride && bIsIniOverride)
				{
					UE_LOG_ASSET_MANAGER(Log, TEXT("Either launch param or DefaultEngine.ini forces activate Tutorial Module: %s"), *ModuleOverride);
				}
				else if (bIsCmdOverride)
				{
					UE_LOG_ASSET_MANAGER(Log, TEXT("Launch param forces activate Tutorial Module: %s"), *ModuleOverride);
				}
				else if (bIsIniOverride)
				{
					UE_LOG_ASSET_MANAGER(Log, TEXT("DefaultEngine.ini forces activate Tutorial Module: %s"), *ModuleOverride);
				}

				RecursiveGetSelfAndDependencyIds(IdsToBeOverriden, TutorialModule);
			}
			else 
			{
				UE_LOG_ASSET_MANAGER(Log, TEXT("Cannot force activate Tutorial Module: %s. Tutorial Module Data Asset is not found."), *ModuleOverride);
			}
		}
		else 
		{
			UE_LOG_ASSET_MANAGER(Log, TEXT("Cannot force activate Tutorial Module: %s. Tutorial Module Data Asset is not found."), *ModuleOverride);
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

		// safeguard to expire the previous one before adding a new notification
		if (OnlineSessionActiveNotificationItem.IsValid())
		{
			OnlineSessionActiveNotificationItem->SetFadeOutDuration(0.0f);
			OnlineSessionActiveNotificationItem->ExpireAndFadeout();
		}

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
