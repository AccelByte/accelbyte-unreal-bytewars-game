// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegionPreferencesModels.h"
#include "RegionPreferencesEssentialsLog.h"
#include "OnlineSessionSettings.h"
#include "RegionPreferencesSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRefreshRegionComplete, bool /* bWasSucceed*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLatencyRefreshComplete, float /* Latency*/)

UCLASS()
class ACCELBYTEWARS_API URegionPreferencesSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void RefreshRegionLatency(bool bRetryOnFailed = false);
	
	TArray<URegionPreferenceInfo*>& GetRegionInfos()
	{
		return RegionInfos;
	}

	TArray<FString> GetEnabledRegion();
	int32 GetEnabledRegionCount();
	bool TryToggleRegion(const FString& RegionCode);
	URegionPreferenceInfo* FindRegionInfo(const FString& RegionCode);

	FString GetCurrentGameSessionRegion();
	bool ShouldShowLatencyInGame();
	void StartLatencyRefreshTimer();
	void StopLatencyRefreshTimer();

	UFUNCTION()
	void RefreshGameLatency() const;

	void FilterSessionSearch(TSharedRef<FOnlineSessionSearch> SessionSearch);
	
	FSimpleMulticastDelegate OnWarningMinimumRegionCount;
	FOnRefreshRegionComplete OnRefreshRegionComplete;
	FOnLatencyRefreshComplete OnLatencyRefreshComplete;
protected:
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() override;
private:
	void GetIngameLatencyRefreshConfiguration();
	void OnGetLatenciesSuccess(const TArray<TPair<FString, float>>& InLatencies);
	void OnGetLatenciesError(int32 ErrorCode, const FString& ErrorMessage);
	
	UFUNCTION()
	void CheatRefreshGameLatency(const TArray<FString>& Args) const;
	
	THandler<TArray<TPair<FString, float>>> OnGetLatenciesSuccessDelegate;	

	UPROPERTY()
	TArray<URegionPreferenceInfo*> RegionInfos;
	
	AccelByte::FApiClientPtr ApiClient;

	int32 NumRetry = 0;
	const FString CommandMyUserInfo = TEXT("ab.region.RefreshGameLatency");
	bool bShowLatency = false;
	int LatencyRefreshInterval = 5;
	FTimerHandle LatencyRefreshTimerHandle;
};
