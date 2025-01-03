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
	
// @@@SNIPSTART PresenceEssentialsSubsystem.h-public
// @@@MULTISNIP GetPresence {"selectedLines": ["1", "5"]}
// @@@MULTISNIP BulkQueryPresence {"selectedLines": ["1", "6"]}
// @@@MULTISNIP SetPresenceStatus {"selectedLines": ["1", "8"]}
// @@@MULTISNIP GetOnPresenceReceivedDelegates {"selectedLines": ["1", "10-13"]}
// @@@MULTISNIP GetOnBulkQueryPresenceCompleteDelegates {"selectedLines": ["1", "15-18"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	void GetPresence(const FUniqueNetIdPtr UserId, bool bForceQuery, const FOnPresenceTaskComplete& OnComplete = FOnPresenceTaskComplete());
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
// @@@SNIPEND

// @@@SNIPSTART PresenceEssentialsSubsystem.h-protected
// @@@MULTISNIP Interface {"selectedLines": ["1", "11-15"]}
// @@@MULTISNIP OnGetPresenceComplete {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnBulkQueryPresenceComplete {"selectedLines": ["1", "3"]}
// @@@MULTISNIP OnSetPresenceStatusComplete {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnPresenceReceived {"selectedLines": ["1", "7"]}
// @@@MULTISNIP UpdatePrimaryPlayerPresenceStatus {"selectedLines": ["1", "9"]}
// @@@MULTISNIP PresenceActivityHelper {"selectedLines": ["1", "18-19"]}
protected:
	void OnGetPresenceComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete);
	void OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences);

	void OnSetPresenceStatusComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete);

	void OnPresenceReceived(const class FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence);

	void UpdatePrimaryPlayerPresenceStatus();

	FOnlinePresenceAccelBytePtr GetPresenceInterface() const;
	TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> GetFriendsInterface() const;
	TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> GetIdentityInterface() const;
	TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> GetSessionInterface() const;
	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;

	// Helper to store presence activity status.
	FString LevelStatus;
	FString ActivityStatus;
// @@@SNIPEND

// @@@SNIPSTART PresenceEssentialsSubsystem.h-private
// @@@MULTISNIP OnLevelLoaded {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnPlayerListChange {"selectedLines": ["1", "3-5"]}
// @@@MULTISNIP GetPrimaryPlayerUserId {"selectedLines": ["1", "7"]}
// @@@MULTISNIP OnPresenceReceivedDelegates {"selectedLines": ["1", "9"]}
// @@@MULTISNIP OnBulkQueryPresenceCompleteDelegates {"selectedLines": ["1", "10"]}
private:
	void OnLevelLoaded();
	void OnFriendListChange();
	void OnBlockedPlayerListChange(int32 LocalUserNum, const FString& ListName);
	void OnSessionParticipantChange(FName SessionName, const FUniqueNetId& UserId, bool bJoined);

	FUniqueNetIdPtr GetPrimaryPlayerUserId();

	FOnPresenceReceived OnPresenceReceivedDelegates;
	FOnBulkQueryPresenceComplete OnBulkQueryPresenceCompleteDelegates;
// @@@SNIPEND
};
