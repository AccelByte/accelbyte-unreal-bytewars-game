// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ByteWarsCore/AssetManager/AccelByteWarsAssetModels.h"
#include "ByteWarsCore/Settings/GameModeDataAssets.h"
#include "AccelByteWarsAssetManager.generated.h"

USTRUCT(BlueprintType)
struct FPrimaryAssetCache
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FPrimaryAssetId, class UAccelByteWarsDataAsset*> AssetMap;

};

class UAccelByteWarsDataAsset;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UAccelByteWarsAssetManager();
	static UAccelByteWarsAssetManager& Get();

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Game Modes"))
	static TArray<FGameModeData> GetAllGameModes();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Game Mode Types"))
	static TArray<FGameModeTypeData> GetAllGameModeTypes();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Tutorial Modules"))
	static TArray<FTutorialModuleData> GetAllTutorialModules();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Assets Type From Cache"))
	TArray<UAccelByteWarsDataAsset*> GetAllAssetsForTypeFromCache(FPrimaryAssetType AssetType) const;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Assets From Cache"))
	UAccelByteWarsDataAsset* GetAssetFromCache(FPrimaryAssetId AssetId) const;

protected:
	//~UAssetManager interface
	virtual void StartInitialLoading() override;
	//~End of UAssetManager interface
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif

	void PopulateAssetCache();

	void LoadAssetsOfType(const TArray<FPrimaryAssetType>& AssetTypes);
	void UnloadAssetsOfType(const TArray<FPrimaryAssetType>& AssetTypes);
	
	void AddAssetToCache(UAccelByteWarsDataAsset* Asset);
	void RemoveAssetFromCache(const FPrimaryAssetId& AssetId);
	
private:

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
	
	TMap<FPrimaryAssetType, FPrimaryAssetCache> PrimaryAssetCache;
};
