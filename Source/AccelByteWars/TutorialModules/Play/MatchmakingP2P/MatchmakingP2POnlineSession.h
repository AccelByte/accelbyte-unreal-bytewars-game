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

// @@@SNIPSTART MatchmakingP2POnlineSession.h-public
// @@@MULTISNIP TargetGameModeMap {"selectedLines": ["1", "15-20"]}
// @@@MULTISNIP StartMatchmaking {"selectedLines": ["1", "22-26"]}
// @@@MULTISNIP CancelMatchmaking {"selectedLines": ["1", "27"]}
// @@@MULTISNIP TravelToSession {"selectedLines": ["1", "6"]}
// @@@MULTISNIP OnStartMatchmakingCompleteDelegate {"selectedLines": ["1", "29-32"]}
// @@@MULTISNIP OnMatchmakingCompleteDelegate {"selectedLines": ["1", "33-36"]}
// @@@MULTISNIP OnCancelMatchmakingCompleteDelegate {"selectedLines": ["1", "37-40"]}
// @@@MULTISNIP OnAcceptBackfillProposalCompleteDelegates {"selectedLines": ["1", "41-44"]}
// @@@MULTISNIP OnSessionServerUpdateReceivedDelegate {"selectedLines": ["1", "8-11"]}
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

#pragma region "Game Session Essentials"
	virtual bool TravelToSession(const FName SessionName) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}
#pragma endregion

#pragma region "Matchmaking Session Essentials"
	const TMap<FString, FString> TargetGameModeMap = {
		{"unreal-elimination-p2p", "ELIMINATION-P2P"},
		{"unreal-teamdeathmatch-p2p", "TEAMDEATHMATCH-P2P"},
		{"unreal-frenzy-elimination-p2p", "FRENZY-ELIMINATION-P2P"},
		{"unreal-frenzy-teamdeathmatch-p2p", "FRENZY-TEAMDEATHMATCH-P2P"}
	};

	virtual void StartMatchmaking(
		const APlayerController* PC,
		const FName& SessionName,
		const EGameModeNetworkType NetworkType,
		const EGameModeType GameModeType, const EGameStyle GameStyle) override;
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
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.h-protected
// @@@MULTISNIP OnStartMatchmakingComplete {"selectedLines": ["1", "13-16"]}
// @@@MULTISNIP OnMatchmakingComplete {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnCancelMatchmakingComplete {"selectedLines": ["1", "17"]}
// @@@MULTISNIP OnSessionServerUpdateReceived {"selectedLines": ["1", "3"]}
// @@@MULTISNIP OnSessionServerErrorReceived {"selectedLines": ["1", "4"]}
// @@@MULTISNIP OnJoinSessionComplete {"selectedLines": ["1", "6"]}
// @@@MULTISNIP OnLeaveSessionComplete {"selectedLines": ["1", "7"]}
// @@@MULTISNIP OnBackfillProposalReceived {"selectedLines": ["1", "20"]}
protected:
#pragma region "Game Session Essentials"
	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
	virtual void OnStartMatchmakingComplete(
		FName SessionName,
		const FOnlineError& ErrorDetails,
		const FSessionMatchmakingResults& Results) override;
	virtual void OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnMatchmakingComplete(FName SessionName, bool bSucceeded) override;

	virtual void OnBackfillProposalReceived(FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal) override;
#pragma endregion 
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2POnlineSession.h-private
// @@@MULTISNIP MatchPoolIdMap {"selectedLines": ["1", "9-14"]}
// @@@MULTISNIP OnLeaveSessionForReMatchmakingComplete {"selectedLines": ["1", "21-26"]}
// @@@MULTISNIP OnStartMatchmakingCompleteDelegate {"selectedLines": ["1", "16"]}
// @@@MULTISNIP OnMatchmakingCompleteDelegate {"selectedLines": ["1", "17"]}
// @@@MULTISNIP OnCancelMatchmakingCompleteDelegate {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnAcceptBackfillProposalCompleteDelegates {"selectedLines": ["1", "19"]}
// @@@MULTISNIP OnSessionServerUpdateReceivedDelegate {"selectedLines": ["1", "5"]}
private:
#pragma region "Game Session Essentials"
	bool bIsInSessionServer = false;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
	const TMap<TPair<EGameModeType, EGameStyle>, FString> MatchPoolIds = {
		{{EGameModeType::FFA, EGameStyle::Zen}, "unreal-elimination-p2p"},
		{{EGameModeType::TDM, EGameStyle::Zen}, "unreal-teamdeathmatch-p2p"},
		{{EGameModeType::FFA, EGameStyle::Frenzy}, "unreal-frenzy-elimination-p2p"},
		{{EGameModeType::TDM, EGameStyle::Frenzy}, "unreal-frenzy-teamdeathmatch-p2p"}
	};

	FOnMatchmakingResponse OnStartMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnMatchmakingCompleteDelegates;
	FOnMatchmakingResponse OnCancelMatchmakingCompleteDelegates;
	FOnMatchmakingAcceptBackfillProposalComplete OnAcceptBackfillProposalCompleteDelegates;

	void OnLeaveSessionForReMatchmakingComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const EGameModeType GameModeType, const EGameStyle GameStyle);
	FDelegateHandle OnLeaveSessionForReMatchmakingCompleteDelegateHandle;
#pragma endregion
// @@@SNIPEND
};
