// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchSessionDSOnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionDSOnlineSession : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

// @@@SNIPSTART MatchSessionDSOnlineSession.h-public
// @@@MULTISNIP QueryUserInfoDeclaration {"selectedLines": ["1", "3-5"]}
// @@@MULTISNIP HelperDelegateDeclaration {"selectedLines": ["1", "9-12", "30-33"]}
// @@@MULTISNIP MatchSessionTemplateNameMap {"selectedLines": ["1", "16-19"]}
// @@@MULTISNIP TravelToSession {"selectedLines": ["1", "7"]}
// @@@MULTISNIP CreateMatchSession {"selectedLines": ["1", "21-24"]}
// @@@MULTISNIP FindSessions {"selectedLines": ["1", "25-28"]}
public:
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

#pragma region "Match Session Essentials"
	const TMap<TPair<EGameModeNetworkType, EGameModeType>, FString> MatchSessionTemplateNameMap = {
		{{EGameModeNetworkType::DS, EGameModeType::FFA}, "unreal-elimination-ds-ams"},
		{{EGameModeNetworkType::DS, EGameModeType::TDM}, "unreal-teamdeathmatch-ds-ams"}
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

// @@@SNIPSTART MatchSessionDSOnlineSession.h-protected
// @@@MULTISNIP QueryUserInfoDeclaration {"selectedLines": ["1", "3-5"]}
// @@@MULTISNIP SessionServerCallbackDeclaration {"selectedLines": ["1", "7-8"]}
// @@@MULTISNIP HandleDisconnectInternal {"selectedLines": ["1", "10"]}
// @@@MULTISNIP OnFindSessionsComplete {"selectedLines": ["1", "14"]}
protected:
#pragma region "Game Session Essentials"
	virtual void OnDSQueryUserInfoComplete(
		const FListBulkUserInfo& UserInfoList,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual void OnSessionServerUpdateReceived(FName SessionName) override;
	virtual void OnSessionServerErrorReceived(FName SessionName, const FString& Message) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;
#pragma endregion

#pragma region "Match Session Essentials"
	virtual void OnFindSessionsComplete(bool bSucceeded) override;
#pragma endregion
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.h-private
// @@@MULTISNIP QueryUserInfoDeclaration {"selectedLines": ["1", "5", "16-18"]}
// @@@MULTISNIP HelperDelegateDeclaration {"selectedLines": ["1", "7", "14"]}
// @@@MULTISNIP HelperVariable {"selectedLines": ["1", "3", "11-12"]}
private:
#pragma region "Game Session Essentials"
	bool bIsInSessionServer = false;

	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

	FOnServerSessionUpdateReceived OnSessionServerUpdateReceivedDelegates;
#pragma endregion

#pragma region "Match Session Essentials"
	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());
	int32 LocalUserNumSearching;

	FOnMatchSessionFindSessionsComplete OnFindSessionsCompleteDelegates;

	void OnQueryUserInfoForFindSessionComplete(
		const ::FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo);
#pragma endregion
// @@@SNIPEND
};
