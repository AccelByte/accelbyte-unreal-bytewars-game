// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "TutorialModules/PartyEssentials/PartyEssentialsLog.h"
#include "PartyOnlineSession_Starter.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPartyOnlineSession_Starter : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()
	
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

	virtual void QueryUserInfo(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete) override;

#pragma region "Party Essentials Module Function Declarations"
	// TODO: Add your party essentials module public function declarations here.
#pragma endregion

protected:
	virtual void OnQueryUserInfoComplete(int32 LocalUserNum, bool bSucceeded, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete) override;

	void OnLeavePartyToTriggerEvent(FName SessionName, bool bSucceeded, const TDelegate<void(bool bWasSuccessful)> OnComplete);

	void InitializePartyGeneratedWidgets();
	void UpdatePartyGeneratedWidgets();
	void DeinitializePartyGeneratedWidgets();
	FUniqueNetIdPtr GetCurrentDisplayedFriendId();

	void OnInviteToPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee);
	void OnKickPlayerFromPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer);
	void OnPromotePartyLeaderButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader);

	UPromptSubsystem* GetPromptSubystem();

#pragma region "Party Essentials Module Function Declarations"
	// TODO: Add your party essentials module protected function declarations here.
#pragma endregion

private:
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	FDelegateHandle OnLeaveSessionForTriggerDelegateHandle;

	FTutorialModuleGeneratedWidget* InviteToPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* KickPlayerFromPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* PromotePartyLeaderButtonMetadata;

#pragma region "Party Essentials Module Properties"
	// TODO: Add your party essentials module properties declaration here.
#pragma endregion
};
