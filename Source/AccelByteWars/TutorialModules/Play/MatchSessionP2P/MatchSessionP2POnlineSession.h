// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchSessionP2POnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionP2POnlineSession : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

// @@@SNIPSTART MatchSessionP2POnlineSession.h-public
// @@@MULTISNIP HelperDelegateDeclaration {"selectedLines": ["1", "5-8", "26-29"]}
// @@@MULTISNIP MatchSessionTemplateNameMap {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP TravelToSession {"selectedLines": ["1", "3"]}
// @@@MULTISNIP CreateMatchSession {"selectedLines": ["1", "17-20"]}
// @@@MULTISNIP FindSessions {"selectedLines": ["1", "21-24"]}
public:
#pragma region "Game Session Essentials"
	virtual bool TravelToSession(const FName SessionName) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}
#pragma endregion

#pragma region "Match Session Essentials"
	const TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTemplateNameMap = {
		{{EGameModeNetworkType::P2P, EGameModeType::FFA}, "unreal-elimination-p2p"},
		{{EGameModeNetworkType::P2P, EGameModeType::TDM}, "unreal-teamdeathmatch-p2p"}
	};

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
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.h-protected
// @@@MULTISNIP SessionServerCallbackDeclaration {"selectedLines": ["1", "3-4", "6", "8"]}
// @@@MULTISNIP HandleDisconnectInternal {"selectedLines": ["1", "10"]}
// @@@MULTISNIP OnLeaveSessionComplete {"selectedLines": ["1", "7"]}
// @@@MULTISNIP OnFindSessionsComplete {"selectedLines": ["1", "14"]}
protected:
#pragma region "Game Session Essentials"
	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;
#pragma endregion

#pragma region "Match Session Essentials"
	virtual void OnFindSessionsComplete(bool bSucceeded) override;
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.h-private
// @@@MULTISNIP QueryUserInfoDeclaration {"selectedLines": ["1", "14-16"]}
// @@@MULTISNIP HelperDelegateDeclaration {"selectedLines": ["1", "5", "12"]}
// @@@MULTISNIP HelperVariable {"selectedLines": ["1", "3", "9-10"]}
private:
#pragma region "Game Session Essentials"
	bool bIsInSessionServer = false;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;
#pragma endregion

#pragma region "Match Session Essentials"
	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
	int32 LocalUserNumSearching;

	FOnMatchSessionFindSessionsComplete OnFindSessionsCompleteDelegates;

	void OnQueryUserInfoForFindSessionComplete(
		const FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo);
#pragma endregion
// @@@SNIPEND
};
