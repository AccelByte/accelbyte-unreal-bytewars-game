// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Play/SessionEssentials/SessionEssentialsOnlineSession.h"
#include "MatchmakingP2POnlineSession_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingP2POnlineSession_Starter : public USessionEssentialsOnlineSession
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

public:
	const TMap<FString, FString> TargetGameModeMap = {
		{"unreal-elimination-p2p", "ELIMINATION-P2P"},
		{"unreal-teamdeathmatch-p2p", "TEAMDEATHMATCH-P2P"},
		{"unreal-frenzy-elimination-p2p", "FRENZY-ELIMINATION-P2P"},
		{"unreal-frenzy-teamdeathmatch-p2p", "FRENZY-TEAMDEATHMATCH-P2P"}
	};

protected:
	virtual bool HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver) override;

private:
	const TMap<TPair<EGameModeType, EGameStyle>, FString> MatchPoolIds = {
		{{EGameModeType::FFA, EGameStyle::Zen}, "unreal-elimination-p2p"},
		{{EGameModeType::TDM, EGameStyle::Zen}, "unreal-teamdeathmatch-p2p"},
		{{EGameModeType::FFA, EGameStyle::Frenzy}, "unreal-frenzy-elimination-p2p"},
		{{EGameModeType::TDM, EGameStyle::Frenzy}, "unreal-frenzy-teamdeathmatch-p2p"}
	};

	bool bIsInSessionServer = false;

#pragma region "Matchmaking with P2P Function Declarations"
public:
	// TODO: Add your module public function declarations here.

protected:
	// TODO: Add your module protected function declarations here.

private:
	// TODO: Add your module private function declarations here.
#pragma endregion 
};
