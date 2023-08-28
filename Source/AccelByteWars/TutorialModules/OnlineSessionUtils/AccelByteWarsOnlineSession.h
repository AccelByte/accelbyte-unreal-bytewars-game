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
	virtual void LeaveSession(FName SessionName) override;

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates() override
	{
		return &OnCreateSessionCompleteDelegates;
	}
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates() override
	{
		return &OnJoinSessionCompleteDelegates;
	}
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates() override
	{
		return &OnLeaveSessionCompleteDelegates;
	}

protected:
	bool bLeaveSessionRunning = false;

	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded) override;
	/*The parent's function with the same name will not be used. Ignore the "hides a non-function" warning*/
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;

private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;

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
	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoComplete& OnComplete) override;
	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual bool TravelToSession(const FName SessionName) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}

protected:
	virtual void OnQueryUserInfoComplete(
		int32 LocalUserNum,
		bool bSucceeded,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FString& ErrorMessage,
		const FOnQueryUsersInfoComplete& OnComplete) override;
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
		{"unreal-teamdeathmatch-ds", "TEAMDEATHMATCH-DS"},
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
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "unreal-elimination-ds"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "unreal-teamdeathmatch-ds"},
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
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "unreal-elimination-ds"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "unreal-teamdeathmatch-ds"},
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
		const bool bSucceeded,
		const TArray<FUserOnlineAccountAccelByte*>& UsersInfo);
#pragma endregion 
};
