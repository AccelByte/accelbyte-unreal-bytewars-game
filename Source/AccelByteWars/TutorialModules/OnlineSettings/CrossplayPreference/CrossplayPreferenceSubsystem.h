// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CrossplayerPreferenceModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AccelByteApiClient.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CrossplayPreferenceSubsystem.generated.h"

class APlayerController;

UCLASS()
class ACCELBYTEWARS_API UCrossplayPreferenceSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void RetrieveCrossplayPreference(const FUniqueNetIdPtr PlayerNetId, FOnRetrieveCrossplayPreferenceCompleted OnComplete);
	void UpdateCrossplayPreference(const FUniqueNetIdPtr PlayerNetId, bool bEnabled, FOnUpdateCrossplayPreferenceCompleted OnComplete);

protected:
	void OnLoginCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

private:
	void SaveCrossplayPreferenceToCloud(const FUniqueNetIdPtr PlayerNetId, bool bEnabled);
	void HandleGetUserRecordCompleted(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FAccelByteModelsUserRecord& UserRecord);
	void ApplyCrossplayPreferenceFromCloud(const FUniqueNetIdPtr& PlayerNetId, bool bCrossplayEnabled);
	APlayerController* FindPlayerControllerByNetId(const FUniqueNetId& PlayerNetId) const;
	int32 GetLocalUserNumFromNetId(const FUniqueNetId& PlayerNetId) const;

	FOnlineSessionV2AccelBytePtr SessionInterface;
	IOnlineIdentityPtr IdentityInterface;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
	FOnlineSessionV2AccelBytePlayerAttributes PlayerAttributes;
	bool bPendingCrossplayInitialize{ true };
};
