// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetManager/AccelByteWarsAssetManager.h"
#include "GameModes/GameModeDataAsset.h"
#include "GameModes/GameModeTypeDataAsset.h"

UAccelByteWarsAssetManager::UAccelByteWarsAssetManager() {

}

void UAccelByteWarsAssetManager::StartInitialLoading() {
	SCOPED_BOOT_TIMING("UAccelByteWarsAssetManager::StartInitialLoading");

	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	// Can do synchronous calls here since this is in boot up

	// Populate game mode Cache on boot up
}

UAccelByteWarsAssetManager& UAccelByteWarsAssetManager::Get() {
	check(GEngine);

	if (UAccelByteWarsAssetManager* Singleton = Cast<UAccelByteWarsAssetManager>(GEngine->AssetManager)) {
		return *Singleton;
	}

	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to PortalWarsAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UAccelByteWarsAssetManager>();
}

#if WITH_EDITOR
// void UAccelByteWarsAssetManager::PreBeginPIE(bool bStartSimulate) {
// 	Super::PreBeginPIE(bStartSimulate);
//
// 	{
// 		FScopedSlowTask SlowTask(0, NSLOCTEXT("PortalWarsEditor", "BeginLoadingPIEData", "Loading Asset Manager Data"));
// 		const bool bShowCancelButton = false;
// 		const bool bAllowInPIE = true;
// 		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
//
// 		// Intentionally after Loading to avoid counting time in this timer
// 		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);
//
// 		// You could add preloading of anything else needed for the experience we'll be using here
// 		// (e.g., by grabbing the default experience from the world settings + the experience override in developer settings)
// 		
// 		// Populate game mode Cache before playing in editor
// 	}
// }
#endif

//////////////////////////////////////////////////////////////////////////
/// GameModes

TArray<FGameModeData> UAccelByteWarsAssetManager::GetAllGameModes()
{
	TArray<FGameModeData> ToReturn;

	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	Get().GetPrimaryAssetIdList(UGameModeDataAsset::GameModeAssetType, PrimaryAssetIdList);

	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIdList) {
		const FString CodeName = UGameModeDataAsset::GetCodeNameFromAssetId(PrimaryAssetId);
		
		FGameModeData ModeData = UGameModeDataAsset::GetGameModeDataByCodeName(CodeName);
		ToReturn.Add(ModeData);
	}

	ToReturn.Sort([](const FGameModeData& LHS, const FGameModeData& RHS) {
		return LHS.DisplayName.ToString() < RHS.DisplayName.ToString();
	});

	return ToReturn;
}

TArray<FGameModeTypeData> UAccelByteWarsAssetManager::GetAllGameModeTypes()
{
	TArray<FGameModeTypeData> ToReturn;

	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	Get().GetPrimaryAssetIdList(UGameModeTypeDataAsset::GameModeTypeAssetType, PrimaryAssetIdList);

	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIdList) {
		FGameModeTypeData TypeData = UGameModeTypeDataAsset::GetGameModeTypeDataForType(PrimaryAssetId);
		ToReturn.Add(TypeData);
	}

	ToReturn.Sort([](const FGameModeTypeData& LHS, const FGameModeTypeData& RHS) {
		return LHS.DisplayName.ToString() < RHS.DisplayName.ToString();
	});

	return ToReturn;
}