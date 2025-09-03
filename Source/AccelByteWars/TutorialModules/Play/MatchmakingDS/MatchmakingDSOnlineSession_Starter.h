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
	virtual void DSQueryUserInfo(
		const TArray<FUniqueNetIdRef>& UserIds,
		const FOnDSQueryUsersInfoComplete& OnComplete) override;

	const TMap<FString, FString> TargetGameModeMap = {
		{"unreal-elimination-ds-ams", "ELIMINATION-DS"},
		{"unreal-teamdeathmatch-ds-ams", "TEAMDEATHMATCH-DS"},
		{"unreal-frenzy-elimination-ds-ams", "FRENZY-ELIMINATION-DS"},
		{"unreal-frenzy-teamdeathmatch-ds-ams", "FRENZY-TEAMDEATHMATCH-DS"}
	};

protected:
	virtual void OnDSQueryUserInfoComplete(const FListUserDataResponse& UserInfoList, const FOnDSQueryUsersInfoComplete& OnComplete) override;

	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;

private:
	const TMap<TPair<EGameModeType, EGameStyle>, FString> MatchPoolIds = {
		{{EGameModeType::FFA, EGameStyle::Zen}, "unreal-elimination-ds-ams"},
		{{EGameModeType::TDM, EGameStyle::Zen}, "unreal-teamdeathmatch-ds-ams"},
		{{EGameModeType::FFA, EGameStyle::Frenzy}, "unreal-frenzy-elimination-ds-ams"},
		{{EGameModeType::TDM, EGameStyle::Frenzy}, "unreal-frenzy-teamdeathmatch-ds-ams"}
	};

	bool bIsInSessionServer = false;

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
