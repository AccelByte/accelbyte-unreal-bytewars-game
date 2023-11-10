// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchSessionP2POnlineSession_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionP2POnlineSession_Starter : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

public:
	const TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTemplateNameMap = {
		{{EGameModeNetworkType::P2P, EGameModeType::FFA}, ""},
		{{EGameModeNetworkType::P2P, EGameModeType::TDM}, ""}
	};

	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoComplete& OnComplete) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}

	virtual FOnMatchSessionFindSessionsComplete* GetOnFindSessionsCompleteDelegates() override
	{
		return &OnFindSessionsCompleteDelegates;
	}

protected:
	virtual void OnQueryUserInfoComplete(
		int32 LocalUserNum,
		bool bSucceeded,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FString& ErrorMessage,
		const FOnQueryUsersInfoComplete& OnComplete) override;

private:
	bool bIsInSessionServer = false;
	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
	int32 LocalUserNumSearching;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;

	FOnMatchSessionFindSessionsComplete OnFindSessionsCompleteDelegates;

	void OnQueryUserInfoForFindSessionComplete(
		const bool bSucceeded,
		const TArray<FUserOnlineAccountAccelByte*>& UsersInfo);

#pragma region "Match Session with P2P declarations"
public:
	// TODO: Add your module public function declarations here.

protected:
	// TODO: Add your module protected function declarations here.
#pragma endregion 
};
