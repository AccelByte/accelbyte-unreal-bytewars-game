// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsOnlineSessionModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSession.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Models/AccelByteSessionModels.h"
#include "AccelByteWarsOnlineSessionBase.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsOnlineSessionBase : public UTutorialModuleOnlineSession
{
	GENERATED_BODY()

protected:
	virtual void JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult) override
	{
		JoinSession(0, SessionName, SearchResult);
	}

#pragma region "Utilities"
protected:
	FOnlineSessionV2AccelBytePtr GetABSessionInt();
	IOnlineIdentityPtr GetIdentityInt() const;
	IOnlineUserPtr GetUserInt() const;

public:
	static int32 GetLocalUserNumFromPlayerController(const APlayerController* PlayerController);
	APlayerController* GetPlayerControllerByUniqueNetId(const FUniqueNetIdPtr UniqueNetId) const;
	APlayerController* GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const;
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;

	bool CompareAccelByteUniqueId(const FUniqueNetIdRepl& FirstUniqueNetId, const FUniqueNetIdRepl& SecondUniqueNetId) const;
	bool CompareAccelByteUniqueId(const FUniqueNetIdRepl& FirstUniqueNetId, const FString& SecondAbUserId) const;
#pragma endregion 

#pragma region "Session Essentials"
public:
	virtual FNamedOnlineSession* GetSession(const FName SessionName){ return nullptr; }
	virtual EAccelByteV2SessionType GetSessionType(const FName SessionName){ return {}; }
	virtual FName GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType){ return FName(); }

	virtual void CreateSession(
		const int32 LocalUserNum,
		FName SessionName,
		FOnlineSessionSettings SessionSettings,
		const EAccelByteV2SessionType SessionType,
		const FString& SessionTemplateName){}
	virtual void JoinSession(const int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& SearchResult){}
	virtual void LeaveSession(FName SessionName){}

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates(){ return nullptr; }
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates(){ return nullptr; }
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates(){ return nullptr; }

protected:
	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded){}
	/*The parent's function with the same name will not be used. Ignore the "hides a non-function" warning*/
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result){}
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded){}
#pragma endregion

#pragma region "Game Session Essentials"
public:
	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoComplete& OnComplete){}
	virtual void DSQueryUserInfo(const TArray<FUniqueNetIdRef>& UserIds, const FOnDSQueryUsersInfoComplete& OnComplete){}

	virtual bool TravelToSession(const FName SessionName){ return false; }

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates(){ return nullptr; }

protected:
	virtual void OnQueryUserInfoComplete(
		int32 LocalUserNum,
		bool bSucceeded,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FString& ErrorMessage,
		const FOnQueryUsersInfoComplete& OnComplete){}
	virtual void OnDSQueryUserInfoComplete(
		const FListBulkUserInfo& UserInfoList,
		const FOnDSQueryUsersInfoComplete& OnComplete){}

	virtual void OnSessionServerUpdateReceived(FName SessionName){}
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message){}

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override{ return false; }

#pragma region "Query caching workaround"
protected:
	bool RetrieveUserInfoCache(
		const TArray<FUniqueNetIdRef>& UserIds,
		TArray<FUserOnlineAccountAccelByte*>& OutUserInfo);
	bool DSRetrieveUserInfoCache(
		const TArray<FUniqueNetIdRef>& UserIds,
		TArray<const FBaseUserInfo*> OutUserInfo);
	void CacheUserInfo(const FPlatformTypes::int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds);
	void CacheUserInfo(const FListBulkUserInfo& UserInfoList);

private:
	TArray<FUserOnlineAccountAccelByte> CachedUsersInfo = {};
	TArray<FBaseUserInfo> DSCachedUsersInfo = {};
#pragma endregion 
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
public:
	virtual void StartMatchmaking(
		const APlayerController* PC,
		const FName& SessionName,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType){}
	virtual void CancelMatchmaking(APlayerController* PC, const FName& SessionName){}

	virtual FOnMatchmakingResponse* GetOnStartMatchmakingCompleteDelegates(){ return nullptr; }
	virtual FOnMatchmakingResponse* GetOnMatchmakingCompleteDelegates(){ return nullptr; }
	virtual FOnMatchmakingResponse* GetOnCancelMatchmakingCompleteDelegates(){ return nullptr; }
	virtual FOnMatchmakingAcceptBackfillProposalComplete* GetOnAcceptBackfillProposalCompleteDelegates(){ return nullptr; }

protected:
	virtual void OnStartMatchmakingComplete(
		FName SessionName,
		const FOnlineError& ErrorDetails,
		const FSessionMatchmakingResults& Results){}
	virtual void OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded){}

	virtual void OnMatchmakingComplete(FName SessionName, bool bSucceeded){}
	virtual void OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal){}

#pragma region "Utilities"
	TArray<FMatchSessionEssentialInfo> SimplifySessionSearchResult(
		const TArray<FOnlineSessionSearchResult>& SearchResults,
		const TArray<FUserOnlineAccountAccelByte*> UsersInfo,
		const TMap<TPair<EGameModeNetworkType, EGameModeType>,
		FString>& SessionTemplateNames);
#pragma endregion 
#pragma endregion

#pragma region "Match Session Essentials"
public:
	virtual void CreateMatchSession(
		const int32 LocalUserNum,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType){}

	virtual void FindSessions(
		const int32 LocalUserNum,
		const int32 MaxQueryNum,
		const bool bForce = false){}

	virtual FOnMatchSessionFindSessionsComplete* GetOnFindSessionsCompleteDelegates(){ return nullptr; }

protected:
	virtual void OnFindSessionsComplete(bool bSucceeded){}
#pragma endregion 

#pragma region "Party Essentials"
public:
	virtual TArray<FUniqueNetIdRef> GetPartyMembers() { return TArray<FUniqueNetIdRef>(); }
	virtual FUniqueNetIdPtr GetPartyLeader() { return nullptr; }
	virtual bool IsInParty(const FUniqueNetIdPtr UserId) { return false; };
	virtual bool IsPartyLeader(const FUniqueNetIdPtr UserId) { return false; };

	virtual void CreateParty(const int32 LocalUserNum) {}
	virtual void LeaveParty(const int32 LocalUserNum) {}

	virtual void SendPartyInvite(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee) {}
	virtual void JoinParty(const int32 LocalUserNum, const FOnlineSessionSearchResult& PartySessionResult) {}
	virtual void RejectPartyInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite) {}

	virtual void KickPlayerFromParty(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer) {}

	virtual void PromotePartyLeader(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader) {}

	virtual FOnCreateSessionComplete* GetOnCreatePartyCompleteDelegates() { return nullptr; }
	virtual FOnDestroySessionComplete* GetOnLeavePartyCompleteDelegates() { return nullptr; }

	virtual FOnSendSessionInviteComplete* GetOnSendPartyInviteCompleteDelegates() { return nullptr; }
	virtual FOnJoinSessionComplete* GetOnJoinPartyCompleteDelegates() { return nullptr; }
	virtual FOnRejectSessionInviteComplete* GetOnRejectPartyInviteCompleteDelegate() { return nullptr; }
	virtual FOnSessionInviteRejected* GetOnPartyInviteRejectedDelegates() { return nullptr; }
	virtual FOnV2SessionInviteReceivedDelegate* GetOnPartyInviteReceivedDelegate() { return nullptr; }

	virtual FOnKickPlayerComplete* GetOnKickPlayerFromPartyDelegate() { return nullptr; }
	virtual FOnKickedFromSession* GetOnKickedFromPartyDelegates() { return nullptr; }

	virtual FOnPromotePartySessionLeaderComplete* GetOnPromotePartyLeaderCompleteDelegate() { return nullptr; }

	virtual FOnSessionParticipantsChange* GetOnPartyMembersChangeDelegates() { return nullptr; }
	virtual FOnSessionUpdateReceived* GetOnPartySessionUpdateReceivedDelegates() { return nullptr; }

protected:
	virtual void OnCreatePartyComplete(FName SessionName, bool bSucceeded) {}
	virtual void OnLeavePartyComplete(FName SessionName, bool bSucceeded) {}

	virtual void OnSendPartyInviteComplete(const FUniqueNetId& Sender, FName SessionName, bool bWasSuccessful, const FUniqueNetId& Invitee) {}
	virtual void OnJoinPartyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {}
	virtual void OnRejectPartyInviteComplete(bool bWasSuccessful) {}
	virtual void OnPartyInviteRejected(FName SessionName, const FUniqueNetId& RejecterId) {}
	virtual void OnPartyInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& PartyInvite) {}

	virtual void OnKickPlayerFromPartyComplete(bool bWasSuccessful, const FUniqueNetId& KickedPlayer) {}
	virtual void OnKickedFromParty(FName SessionName) {}

	virtual void OnPromotePartyLeaderComplete(const FUniqueNetId& NewLeader, const FOnlineError& Result) {}

	virtual void OnPartyMembersChange(FName SessionName, const FUniqueNetId& Member, bool bJoined) {}
	virtual void OnPartySessionUpdateReceived(FName SessionName) {}
#pragma endregion

#pragma region "Playing With Party"
public:
	TDelegate<bool()> ValidateToStartSession;
	TDelegate<bool(const EGameModeType GameModeType)> ValidateToStartMatchmaking;
	TDelegate<bool(const FOnlineSessionSearchResult& SessionSearchResult)> ValidateToJoinSession;
#pragma endregion
};
