// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "PartyEssentialsLog.h"
#include "PartyOnlineSession.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPartyOnlineSession : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()
	
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;
	
	virtual void QueryUserInfo(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete) override;

#pragma region "Party Essentials Module Function Declarations"

	virtual TArray<FUniqueNetIdRef> GetPartyMembers() override;
	virtual FUniqueNetIdPtr GetPartyLeader() override;
	virtual bool IsInParty(const FUniqueNetIdPtr UserId);
	virtual bool IsPartyLeader(const FUniqueNetIdPtr UserId) override;

	virtual void CreateParty(const int32 LocalUserNum) override;
	virtual void LeaveParty(const int32 LocalUserNum) override;

	virtual void SendPartyInvite(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee) override;
	virtual void JoinParty(const int32 LocalUserNum, const FOnlineSessionSearchResult& PartySessionResult) override;
	virtual void RejectPartyInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite) override;

	virtual void KickPlayerFromParty(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer) override;

	virtual void PromotePartyLeader(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader) override;

	virtual FOnCreateSessionComplete* GetOnCreatePartyCompleteDelegates() 
	{ 
		return &OnCreatePartyCompleteDelegates; 
	}
	virtual FOnDestroySessionComplete* GetOnLeavePartyCompleteDelegates() 
	{ 
		return &OnLeavePartyCompleteDelegates; 
	}

	virtual FOnSendSessionInviteComplete* GetOnSendPartyInviteCompleteDelegates() override
	{
		return &OnSendPartyInviteCompleteDelegates;;
	}
	virtual FOnJoinSessionComplete* GetOnJoinPartyCompleteDelegates()
	{
		return &OnJoinPartyCompleteDelegates;
	}
	virtual FOnRejectSessionInviteComplete* GetOnRejectPartyInviteCompleteDelegate() override
	{ 
		return &OnRejectPartyInviteCompleteDelegate;
	}
	virtual FOnSessionInviteRejected* GetOnPartyInviteRejectedDelegates() override
	{
		return &OnPartyInviteRejectedDelegates;
	}
	virtual FOnV2SessionInviteReceivedDelegate* GetOnPartyInviteReceivedDelegate() override
	{
		return &OnPartyInviteReceivedDelegate;
	}

	virtual FOnKickPlayerComplete* GetOnKickPlayerFromPartyDelegate() override
	{
		return &OnKickPlayerFromPartyCompleteDelegate;
	}
	virtual FOnKickedFromSession* GetOnKickedFromPartyDelegates() override
	{
		return &OnKickedFromPartyDelegates;
	}

	virtual FOnPromotePartySessionLeaderComplete* GetOnPromotePartyLeaderCompleteDelegate() override
	{ 
		return &OnPromotePartyLeaderCompleteDelegate;
	}

	virtual FOnSessionParticipantsChange* GetOnPartyMembersChangeDelegates() override
	{
		return &OnPartyMembersChangeDelegates;
	}
	virtual FOnSessionUpdateReceived* GetOnPartySessionUpdateReceivedDelegates() override
	{ 
		return &OnPartySessionUpdateReceivedDelegates; 
	}

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

	virtual void OnCreatePartyComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnLeavePartyComplete(FName SessionName, bool bSucceeded) override;

	virtual void OnSendPartyInviteComplete(const FUniqueNetId& Sender, FName SessionName, bool bWasSuccessful, const FUniqueNetId& Invitee) override;
	virtual void OnJoinPartyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnRejectPartyInviteComplete(bool bWasSuccessful) override;
	virtual void OnPartyInviteRejected(FName SessionName, const FUniqueNetId& RejecterId) override;
	virtual void OnPartyInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& PartyInvite) override;
	void DisplayJoinPartyConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite);

	virtual void OnKickPlayerFromPartyComplete(bool bWasSuccessful, const FUniqueNetId& KickedPlayer) override;
	virtual void OnKickedFromParty(FName SessionName) override;

	virtual void OnPromotePartyLeaderComplete(const FUniqueNetId& NewLeader, const FOnlineError& Result) override;
	void DisplayCurrentPartyLeader();

	virtual void OnPartyMembersChange(FName SessionName, const FUniqueNetId& Member, bool bJoined) override;
	virtual void OnPartySessionUpdateReceived(FName SessionName) override;

	void LeaveRestoredPartyToTriggerEvent(const FUniqueNetId& LocalUserId, const FOnlineError& Result, const TDelegate<void(bool bSucceeded)> OnComplete);
	void OnLeaveRestoredPartyToTriggerEventComplete(bool bSucceeded, FString SessionId, const TDelegate<void(bool bSucceeded)> OnComplete);

	void OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error);

#pragma endregion

private:
	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	FDelegateHandle OnLeaveSessionForTriggerDelegateHandle;

	FTutorialModuleGeneratedWidget* InviteToPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* KickPlayerFromPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* PromotePartyLeaderButtonMetadata;

#pragma region "Party Essentials Module Properties"
	const FString PartySessionTemplate = FString("unreal-party");
	FUniqueNetIdPtr LastPartyLeader;
	/* Helper variable to cache party member status. */
	TMap<FString, bool> PartyMemberStatus;

	FOnCreateSessionComplete OnCreatePartyCompleteDelegates;
	FOnDestroySessionComplete OnLeavePartyCompleteDelegates;

	FOnSendSessionInviteComplete OnSendPartyInviteCompleteDelegates;
	FOnJoinSessionComplete OnJoinPartyCompleteDelegates;
	FOnRejectSessionInviteComplete OnRejectPartyInviteCompleteDelegate;
	FOnSessionInviteRejected OnPartyInviteRejectedDelegates;
	FOnV2SessionInviteReceivedDelegate OnPartyInviteReceivedDelegate;

	FOnKickPlayerComplete OnKickPlayerFromPartyCompleteDelegate;
	FOnKickedFromSession OnKickedFromPartyDelegates;

	FOnPromotePartySessionLeaderComplete OnPromotePartyLeaderCompleteDelegate;

	FOnSessionParticipantsChange OnPartyMembersChangeDelegates;
	FOnSessionUpdateReceived OnPartySessionUpdateReceivedDelegates;
#pragma endregion
};
