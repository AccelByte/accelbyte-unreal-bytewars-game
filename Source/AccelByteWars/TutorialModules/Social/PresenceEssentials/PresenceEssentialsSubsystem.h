// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "PresenceEssentialsLog.h"
#include "PresenceEssentialsModels.h"
#include "PresenceEssentialsSubsystem.generated.h"

class FOnlineFriendsAccelByte;
class FOnlineIdentityAccelByte;
class UAccelByteWarsOnlineSessionBase;

UCLASS()
class ACCELBYTEWARS_API UPresenceEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	void GetPresence(const FUniqueNetIdPtr UserId, const FOnPresenceTaskComplete& OnComplete = FOnPresenceTaskComplete());
	void BulkQueryPresence(const FUniqueNetIdPtr UserId, const TArray<FUniqueNetIdRef>& UserIds);

	void SetPresenceStatus(const FUniqueNetIdPtr UserId, const FString& Status, const FOnPresenceTaskComplete& OnComplete = FOnPresenceTaskComplete());

	FOnPresenceReceived* GetOnPresenceReceivedDelegates()
	{
		return &OnPresenceReceivedDelegates;
	}

	FOnBulkQueryPresenceComplete* GetOnBulkQueryPresenceCompleteDelegates()
	{
		return &OnBulkQueryPresenceCompleteDelegates;
	}

protected:
	void OnGetPresenceComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete);
	void OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences);

	void OnSetPresenceStatusComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete);

	void OnPresenceReceived(const class FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence);

	void UpdatePrimaryPlayerPresenceStatus();

	FOnlinePresenceAccelBytePtr GetPresenceInterface() const;
	TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> GetFriendsInterface() const;
	TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> GetIdentityInterface() const;
	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;

	// Helper to store presence activity status.
	FString LevelStatus;
	FString ActivityStatus;

private:
	void OnLevelLoaded();
	void OnFriendListChange();
	void OnBlockedPlayerListChange(int32 LocalUserNum, const FString& ListName);

	FUniqueNetIdPtr GetPrimaryPlayerUserId();

	FOnPresenceReceived OnPresenceReceivedDelegates;
	FOnBulkQueryPresenceComplete OnBulkQueryPresenceCompleteDelegates;
};
