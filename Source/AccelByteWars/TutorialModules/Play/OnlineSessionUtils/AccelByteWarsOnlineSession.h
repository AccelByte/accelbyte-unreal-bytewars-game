// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsOnlineSessionBase.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSession.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Models/AccelByteSessionModels.h"
#include "AccelByteWarsOnlineSession.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsOnlineSession final : public UAccelByteWarsOnlineSessionBase
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

protected:
	virtual void JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult) override
	{
		JoinSession(0, SessionName, SearchResult);
	}

#pragma region "Session Essentials"
public:
	virtual FNamedOnlineSession* GetSession(const FName SessionName) override;
	virtual EAccelByteV2SessionType GetSessionType(const FName SessionName) override;
	virtual FName GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType) override;

	virtual void CreateSession(
		const int32 LocalUserNum,
		FName SessionName,
		FOnlineSessionSettings SessionSettings,
		const EAccelByteV2SessionType SessionType,
		const FString& SessionTemplateName) override;
	virtual void JoinSession(const int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& SearchResult) override;
	virtual void SendSessionInvite(const int32 LocalUserNum, FName SessionName, const FUniqueNetIdPtr Invitee) override;
	virtual void RejectSessionInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& Invite) override;
	virtual void LeaveSession(FName SessionName) override;
	virtual void UpdateSessionJoinability(const FName SessionName, const EAccelByteV2SessionJoinability Joinability) override;

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates() override
	{
		return &OnCreateSessionCompleteDelegates;
	}
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates() override
	{
		return &OnJoinSessionCompleteDelegates;
	}
	virtual FOnSendSessionInviteComplete* GetOnSendSessionInviteCompleteDelegates() override
	{
		return &OnSendSessionInviteCompleteDelegates;
	}
	virtual FOnRejectSessionInviteCompleteMulticast* GetOnRejectSessionInviteCompleteDelegate() override
	{
		return &OnRejectSessionInviteCompleteDelegates;
	}
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates() override
	{
		return &OnLeaveSessionCompleteDelegates;
	}
	virtual FOnUpdateSessionComplete* GetOnUpdateSessionCompleteDelegates() override
	{
		return &OnUpdateSessionCompleteDelegates;
	}

	virtual FOnV2SessionInviteReceived* GetOnSessionInviteReceivedDelegates() override
	{
		return &OnSessionInviteReceivedDelegates;
	}
	virtual FOnSessionParticipantsChange* GetOnSessionParticipantsChange() override
	{
		return &OnSessionParticipantsChangeDelegates;
	}

protected:
	bool bLeaveSessionRunning = false;

	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded) override;
	/*The parent's function with the same name will not be used. Ignore the "hides a non-function" warning*/
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnSendSessionInviteComplete(
		const FUniqueNetId& LocalSenderId,
		FName SessionName,
		bool bSucceeded,
		const FUniqueNetId& InviteeId) override;
	virtual void OnRejectSessionInviteComplete(bool bSucceeded) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnUpdateSessionComplete(FName SessionName, bool bSucceeded) override;

	virtual void OnSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FOnlineSessionInviteAccelByte& Invite) override;
	virtual void OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member, bool bJoined) override;

private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnSendSessionInviteComplete OnSendSessionInviteCompleteDelegates;
	FOnRejectSessionInviteCompleteMulticast OnRejectSessionInviteCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;
	FOnUpdateSessionComplete OnUpdateSessionCompleteDelegates;

	FOnV2SessionInviteReceived OnSessionInviteReceivedDelegates;
	FOnSessionParticipantsChange OnSessionParticipantsChangeDelegates;

	void OnLeaveSessionForCreateSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSettings SessionSettings);
	FDelegateHandle OnLeaveSessionForCreateSessionCompleteDelegateHandle;

	void OnLeaveSessionForJoinSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSearchResult SearchResult);
	FDelegateHandle OnLeaveSessionForJoinSessionCompleteDelegateHandle;
#pragma endregion

#pragma region "Game Session Essentials"
public:
	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual bool TravelToSession(const FName SessionName) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}

protected:
	virtual void OnDSQueryUserInfoComplete(
		const FListBulkUserInfo& UserInfoList,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;

private:
	bool bIsInSessionServer = false;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;

#pragma endregion

#pragma region "Matchmaking Session Essentials"
public:
	TMap<FString, FString> MatchmakingTargetGameModeMap = {
		{"unreal-elimination-ds", "ELIMINATION-DS"},
		{"unreal-elimination-ds-ams", "ELIMINATION-DS"},
		{"unreal-teamdeathmatch-ds", "TEAMDEATHMATCH-DS"},
		{"unreal-teamdeathmatch-ds-ams", "TEAMDEATHMATCH-DS"},
		{"unreal-elimination-p2p", "ELIMINATION-P2P"},
		{"unreal-teamdeathmatch-p2p", "TEAMDEATHMATCH-P2P"}
	};

	virtual void StartMatchmaking(
		const APlayerController* PC,
		const FName& SessionName,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType) override;
	virtual void CancelMatchmaking(APlayerController* PC, const FName& SessionName) override;

	virtual FOnMatchmakingResponse* GetOnStartMatchmakingCompleteDelegates() override
	{
		return &OnStartMatchmakingCompleteDelegates;
	}
	virtual FOnMatchmakingResponse* GetOnMatchmakingCompleteDelegates() override
	{
		return &OnMatchmakingCompleteDelegates;
	}
	virtual FOnMatchmakingResponse* GetOnCancelMatchmakingCompleteDelegates() override
	{
		return &OnCancelMatchmakingCompleteDelegates;
	}
	virtual FOnMatchmakingAcceptBackfillProposalComplete* GetOnAcceptBackfillProposalCompleteDelegates() override
	{
		return &OnAcceptBackfillProposalCompleteDelegates;
	}

protected:
	virtual void OnStartMatchmakingComplete(
		FName SessionName,
		const FOnlineError& ErrorDetails,
		const FSessionMatchmakingResults& Results) override;
	virtual void OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) override;

	virtual void OnMatchmakingComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal) override;

private:
	TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchmakingPoolIdMap = {
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "unreal-elimination-ds-ams"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "unreal-teamdeathmatch-ds-ams"},
		{{EGameModeNetworkType::P2P, EGameModeType::FFA}, "unreal-elimination-p2p"},
		{{EGameModeNetworkType::P2P, EGameModeType::TDM}, "unreal-teamdeathmatch-p2p"}
	};

	FOnMatchmakingResponse OnStartMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnCancelMatchmakingCompleteDelegates;
	FOnMatchmakingAcceptBackfillProposalComplete OnAcceptBackfillProposalCompleteDelegates;

	void OnLeaveSessionForReMatchmakingComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType);
	FDelegateHandle OnLeaveSessionForReMatchmakingCompleteDelegateHandle;
#pragma endregion

#pragma region "Match Session Essentials"
public:
	virtual void CreateMatchSession(
		const int32 LocalUserNum,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType) override;
	virtual void FindSessions(
		const int32 LocalUserNum,
		const int32 MaxQueryNum,
		const bool bForce) override;

	virtual FOnMatchSessionFindSessionsComplete* GetOnFindSessionsCompleteDelegates() override
	{
		return &OnFindSessionsCompleteDelegates;
	}

protected:
	virtual void OnFindSessionsComplete(bool bSucceeded) override;

private:
	TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTemplateNameMap = {
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "unreal-elimination-ds-ams"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "unreal-teamdeathmatch-ds-ams"},
		{{EGameModeNetworkType::P2P, EGameModeType::FFA}, "unreal-elimination-p2p"},
		{{EGameModeNetworkType::P2P, EGameModeType::TDM}, "unreal-teamdeathmatch-p2p"}
	};
	TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTargetGameModeMap = {
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "ELIMINATION-DS-USERCREATED"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "TEAMDEATHMATCH-DS-USERCREATED"},
		{{EGameModeNetworkType::P2P, EGameModeType::FFA}, "ELIMINATION-P2P-USERCREATED"},
		{{EGameModeNetworkType::P2P, EGameModeType::TDM}, "TEAMDEATHMATCH-P2P-USERCREATED"}
	};

	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
	int32 LocalUserNumSearching;

	FOnMatchSessionFindSessionsComplete OnFindSessionsCompleteDelegates;

	void OnQueryUserInfoForFindSessionComplete(
		const FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo);
#pragma endregion 

#pragma region "Party Essentials"
public:
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

private:
	FDelegateHandle OnCreatePartyToInviteMemberDelegateHandle;
	FDelegateHandle OnLeaveSessionForTriggerDelegateHandle;

	FTutorialModuleGeneratedWidget* InviteToPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* KickPlayerFromPartyButtonMetadata;
	FTutorialModuleGeneratedWidget* PromotePartyLeaderButtonMetadata;

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

#pragma region "Lobby Essentials"
protected:
	void OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error);
	void OnLobbyReconnecting(int32 LocalUserNum, const FUniqueNetId& UserId, int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnLobbyReconnected();
	void OnLobbyConnectionClosed(int32 LocalUserNum, const FUniqueNetId& UserId, int32 StatusCode, const FString& Reason, bool bWasClean);
#pragma endregion
};
