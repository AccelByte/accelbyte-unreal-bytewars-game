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
	
// @@@SNIPSTART PartyOnlineSession.h-public
// @@@MULTISNIP PartyUtility {"selectedLines": ["1", "6-9"]}
// @@@MULTISNIP CreateParty {"selectedLines": ["1", "11"]}
// @@@MULTISNIP LeaveParty {"selectedLines": ["1", "12"]}
// @@@MULTISNIP SendPartyInvite {"selectedLines": ["1", "14"]}
// @@@MULTISNIP JoinParty {"selectedLines": ["1", "15"]}
// @@@MULTISNIP RejectPartyInvite {"selectedLines": ["1", "16"]}
// @@@MULTISNIP KickPlayerFromParty {"selectedLines": ["1", "18"]}
// @@@MULTISNIP PromotePartyLeader {"selectedLines": ["1", "20"]}
// @@@MULTISNIP OnCreatePartyCompleteDelegate {"selectedLines": ["1", "22-25"]}
// @@@MULTISNIP OnLeavePartyCompleteDelegate {"selectedLines": ["1", "26-29"]}
// @@@MULTISNIP OnSendPartyInviteCompleteDelegate {"selectedLines": ["1", "31-34"]}
// @@@MULTISNIP OnJoinPartyCompleteDelegate {"selectedLines": ["1", "35-38"]}
// @@@MULTISNIP OnRejectPartyInviteCompleteDelegate {"selectedLines": ["1", "39-42"]}
// @@@MULTISNIP OnPartyInviteRejectedDelegate {"selectedLines": ["1", "43-46"]}
// @@@MULTISNIP OnPartyInviteReceivedDelegate {"selectedLines": ["1", "47-50"]}
// @@@MULTISNIP OnKickPlayerFromPartyCompleteDelegate {"selectedLines": ["1", "52-55"]}
// @@@MULTISNIP OnKickedFromPartyDelegate {"selectedLines": ["1", "56-59"]}
// @@@MULTISNIP OnPromotePartyLeaderCompleteDelegate {"selectedLines": ["1", "61-64"]}
// @@@MULTISNIP OnPartySessionUpdateDelegate {"selectedLines": ["1", "66-73"]}
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

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
// @@@SNIPEND

// @@@SNIPSTART PartyOnlineSession.h-protected
// @@@MULTISNIP OnCreatePartyComplete {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnCreatePartyToInviteMember {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnLeavePartyComplete {"selectedLines": ["1", "19"]}
// @@@MULTISNIP OnSendPartyInviteComplete {"selectedLines": ["1", "21"]}
// @@@MULTISNIP OnJoinPartyComplete {"selectedLines": ["1", "22"]}
// @@@MULTISNIP OnRejectPartyInviteComplete {"selectedLines": ["1", "23"]}
// @@@MULTISNIP OnPartyInviteRejected {"selectedLines": ["1", "24"]}
// @@@MULTISNIP OnPartyInviteReceived {"selectedLines": ["1", "25"]}
// @@@MULTISNIP DisplayJoinPartyConfirmation {"selectedLines": ["1", "26"]}
// @@@MULTISNIP OnKickPlayerFromPartyComplete {"selectedLines": ["1", "28"]}
// @@@MULTISNIP OnKickedFromParty {"selectedLines": ["1", "29"]}
// @@@MULTISNIP OnPromotePartyLeaderComplete {"selectedLines": ["1", "31"]}
// @@@MULTISNIP DisplayCurrentPartyLeader {"selectedLines": ["1", "32"]}
// @@@MULTISNIP OnPartySessionUpdate {"selectedLines": ["1", "34-35"]}
// @@@MULTISNIP OnConnectLobbyComplete {"selectedLines": ["1", "37"]}
protected:
	void OnCreatePartyToInviteMember(FName SessionName, bool bWasSuccessful, const int32 LocalUserNum, const FUniqueNetIdPtr SenderId, const FUniqueNetIdPtr InviteeId);

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

	void OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error);
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART PartyOnlineSession.h-private
// @@@MULTISNIP OnCreatePartyCompleteDelegate {"selectedLines": ["1", "15"]}
// @@@MULTISNIP OnCreatePartyToInviteMemberDelegate {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnLeavePartyCompleteDelegate {"selectedLines": ["1", "16"]}
// @@@MULTISNIP OnSendPartyInviteCompleteDelegate {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnJoinPartyCompleteDelegate {"selectedLines": ["1", "19"]}
// @@@MULTISNIP OnRejectPartyInviteCompleteDelegate {"selectedLines": ["1", "20"]}
// @@@MULTISNIP OnPartyInviteRejectedDelegate {"selectedLines": ["1", "21"]}
// @@@MULTISNIP OnPartyInviteReceivedDelegate {"selectedLines": ["1", "22"]}
// @@@MULTISNIP OnKickPlayerFromPartyCompleteDelegate {"selectedLines": ["1", "24"]}
// @@@MULTISNIP OnKickedFromPartyDelegate {"selectedLines": ["1", "25"]}
// @@@MULTISNIP OnPromotePartyLeaderCompleteDelegate {"selectedLines": ["1", "27"]}
// @@@MULTISNIP OnPartySessionUpdateDelegate {"selectedLines": ["1", "29-30"]}
// @@@MULTISNIP PartySessionTemplate {"selectedLines": ["1", "10"]}
// @@@MULTISNIP LastPartyLeader {"selectedLines": ["1", "11"]}
// @@@MULTISNIP PartyMemberStatus {"selectedLines": ["1", "13"]}
private:
	FDelegateHandle OnCreatePartyToInviteMemberDelegateHandle;
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
// @@@SNIPEND
};
