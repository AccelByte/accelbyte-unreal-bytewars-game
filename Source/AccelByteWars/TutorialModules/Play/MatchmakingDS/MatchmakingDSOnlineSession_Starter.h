// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchmakingDSOnlineSession_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingDSOnlineSession_Starter : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()
	
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

public:
	virtual void QueryUserInfo(
		const int32 LocalUserNum,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnQueryUsersInfoComplete& OnComplete) override;
	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	const TMap<FString, FString> TargetGameModeMap = {
		{"unreal-elimination-ds", "ELIMINATION-DS"},
		{"unreal-teamdeathmatch-ds", "TEAMDEATHMATCH-DS"}
	};

protected:
	virtual void OnQueryUserInfoComplete(
		int32 LocalUserNum,
		bool bSucceeded,
		const TArray<FUniqueNetIdRef>& UserIds,
		const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete) override;
	virtual void OnDSQueryUserInfoComplete(const FListBulkUserInfo& UserInfoList, const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;

private:
	const TMap<EGameModeType, FString> MatchPoolIds = {
		{EGameModeType::FFA, "unreal-elimination-ds"},
		{EGameModeType::TDM, "unreal-teamdeathmatch-ds"}
	};

	bool bIsInSessionServer = false;

	FDelegateHandle OnQueryUserInfoCompleteDelegateHandle;
	FDelegateHandle OnDSQueryUserInfoCompleteDelegateHandle;

#pragma region "Matchmaking with DS Function Declarations"
public:
	// TODO: Add your module public function declarations here.

protected:
	// TODO: Add your module protected function declarations here.

private:
	// TODO: Add your module private function declarations here.
#pragma endregion 
};
