// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchmakingDSOnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingDSOnlineSession final : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

// @@@SNIPSTART MatchmakingDSOnlineSession.h-public
// @@@MULTISNIP TargetGameModeMap {"selectedLines": ["1", "19-22"]}
// @@@MULTISNIP StartMatchmaking {"selectedLines": ["1", "24-28"]}
// @@@MULTISNIP CancelMatchmaking {"selectedLines": ["1", "29"]}
// @@@MULTISNIP TravelToSession {"selectedLines": ["1", "10"]}
// @@@MULTISNIP OnStartMatchmakingCompleteDelegate {"selectedLines": ["1", "31-34"]}
// @@@MULTISNIP OnMatchmakingCompleteDelegate {"selectedLines": ["1", "35-38"]}
// @@@MULTISNIP OnCancelMatchmakingCompleteDelegate {"selectedLines": ["1", "39-42"]}
// @@@MULTISNIP OnAcceptBackfillProposalCompleteDelegates {"selectedLines": ["1", "43-46"]}
// @@@MULTISNIP OnSessionServerUpdateReceivedDelegate {"selectedLines": ["1", "12-15"]}
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

#pragma region "Game Session Essentials"
	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual bool TravelToSession(const FName SessionName) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}
#pragma endregion

#pragma region "Matchmaking Session Essentials"
	const TMap<FString, FString> TargetGameModeMap = {
		{"unreal-elimination-ds-ams", "ELIMINATION-DS"},
		{"unreal-teamdeathmatch-ds-ams", "TEAMDEATHMATCH-DS"}
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
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSOnlineSession.h-protected
// @@@MULTISNIP OnStartMatchmakingComplete {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP OnMatchmakingComplete {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnCancelMatchmakingComplete {"selectedLines": ["1", "16"]}
// @@@MULTISNIP OnSessionServerUpdateReceived {"selectedLines": ["1", "5"]}
// @@@MULTISNIP OnSessionServerErrorReceived {"selectedLines": ["1", "6"]}
// @@@MULTISNIP OnBackfillProposalReceived {"selectedLines": ["1", "19"]}
protected:
#pragma region "Game Session Essentials"
	virtual void OnDSQueryUserInfoComplete(const FListBulkUserInfo& UserInfoList, const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

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

// @@@SNIPSTART MatchmakingDSOnlineSession.h-private
// @@@MULTISNIP MatchPoolIdMap {"selectedLines": ["1", "11-14"]}
// @@@MULTISNIP OnLeaveSessionForReMatchmakingComplete {"selectedLines": ["1", "21-26"]}
// @@@MULTISNIP OnStartMatchmakingCompleteDelegate {"selectedLines": ["1", "16"]}
// @@@MULTISNIP OnMatchmakingCompleteDelegate {"selectedLines": ["1", "17"]}
// @@@MULTISNIP OnCancelMatchmakingCompleteDelegate {"selectedLines": ["1", "18"]}
// @@@MULTISNIP OnAcceptBackfillProposalCompleteDelegates {"selectedLines": ["1", "19"]}
// @@@MULTISNIP OnSessionServerUpdateReceivedDelegate {"selectedLines": ["1", "7"]}
private:
#pragma region "Game Session Essentials"
	bool bIsInSessionServer = false;

	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
	const TMap<EGameModeType, FString> MatchPoolIds = {
		{EGameModeType::FFA, "unreal-elimination-ds-ams"},
		{EGameModeType::TDM, "unreal-teamdeathmatch-ds-ams"}
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
// @@@SNIPEND
};
