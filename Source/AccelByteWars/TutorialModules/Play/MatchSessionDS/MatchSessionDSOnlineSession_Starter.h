// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchSessionDSOnlineSession_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionDSOnlineSession_Starter : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

public:
	const TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTemplateNameMap = {
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, ""},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, ""}
	};

	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual FOnServerSessionUpdateReceived* GetOnSessionServerUpdateReceivedDelegates() override
	{
		return &OnSessionServerUpdateReceivedDelegates;
	}

	virtual FOnMatchSessionFindSessionsComplete* GetOnFindSessionsCompleteDelegates() override
	{
		return &OnFindSessionsCompleteDelegates;
	}

protected:
	virtual void OnDSQueryUserInfoComplete(
		const FListBulkUserInfo& UserInfoList,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

private:
	bool bIsInSessionServer = false;
	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
	int32 LocalUserNumSearching;

	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;

	FOnMatchSessionFindSessionsComplete OnFindSessionsCompleteDelegates;

	void OnQueryUserInfoForFindSessionComplete(
		const ::FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo);

#pragma region "Match Session with DS declarations"
public:
	// TODO: Add your module public function declarations here.

protected:
	// TODO: Add your module protected function declarations here.
#pragma endregion
};
