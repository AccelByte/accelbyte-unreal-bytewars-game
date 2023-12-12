// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Core/AssetManager/AccelByteWarsAssetModels.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "AccelByteWarsAssetManager.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsAssetManager, Log, All);
#define UE_LOG_ASSET_MANAGER(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsAssetManager, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

class UTutorialModuleDataAsset;

USTRUCT(BlueprintType)
struct FPrimaryAssetCache
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FPrimaryAssetId, class UAccelByteWarsDataAsset*> AssetMap;

};

class UAccelByteWarsDataAsset;

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

#pragma region "Online Session"
public:
	/**
	 * @brief Get the complete Online Session class from Tutorial Modules
	 *
	 * This function will store the CompleteOnlineSession class and will return that stored value if possible.
	 * Minimizing the number of iteration needed if this function needs to be called multiple times.
	 */
	TSubclassOf<UOnlineSession> GetCompleteOnlineSessionClassFromDataAsset();

	/**
	 * @brief Get the supposed current online session class
	 */
	TSubclassOf<UOnlineSession> GetPreferredOnlineSessionClassFromDataAsset();

private:
	const FString OnlineSessionCompleteTutorialModuleCodeName = "ONLINESESSIONCOMPLETE";
	TSubclassOf<UOnlineSession> CompleteOnlineSessionClass;
	TSubclassOf<UOnlineSession> PreferredOnlineSessionClass;
#pragma endregion 

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

	void TutorialModuleOverride();
	void StarterOnlineSessionModulesChecker();

#if UE_EDITOR
	virtual void PostInitialAssetScan() override;

	TSharedPtr<SNotificationItem> OverrideNotificationItem;
	TSharedPtr<SNotificationItem> OnlineSessionActiveNotificationItem;
#endif

private:

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
	
	TMap<FPrimaryAssetType, FPrimaryAssetCache> PrimaryAssetCache;

	static void RecursiveGetSelfAndDependencyIds(TArray<FString>& OutIds, const UTutorialModuleDataAsset* TutorialModule);
};
