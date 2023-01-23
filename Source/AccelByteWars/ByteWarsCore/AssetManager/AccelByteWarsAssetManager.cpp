// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/AssetManager/AccelByteWarsAssetManager.h"
#include "GameModes/GameModeDataAsset.h"
#include "GameModes/GameModeTypeDataAsset.h"
#include "TutorialModules/TutorialModuleDataAsset.h"

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