// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchmakingP2POnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingP2POnlineSession : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

protected:
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;

#pragma region "Game Session Essentials"
public:
	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoComplete& OnComplete) override;

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

	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;

private:
	bool bIsInSessionServer = false;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
public:
	const TMap<FString, FString> TargetGameModeMap = {
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
	const TMap<EGameModeType, FString> MatchPoolIds = {
		{EGameModeType::FFA, "unreal-elimination-p2p"},
		{EGameModeType::TDM, "unreal-teamdeathmatch-p2p"}
	};

	FOnMatchmakingResponse OnStartMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnCancelMatchmakingCompleteDelegates;
	FOnMatchmakingAcceptBackfillProposalComplete OnAcceptBackfillProposalCompleteDelegates;

	void OnLeaveSessionForReMatchmakingComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const EGameModeType GameModeType);
	FDelegateHandle OnLeaveSessionForReMatchmakingCompleteDelegateHandle;
#pragma endregion 
};
