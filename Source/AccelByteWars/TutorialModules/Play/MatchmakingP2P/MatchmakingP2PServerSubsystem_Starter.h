// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchmakingP2POnlineSession_Starter.h"
#include "Play/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchmakingP2PServerSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingP2PServerSubsystem_Starter : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region "Matchmaking with P2P Function Declarations"
protected:
	// TODO: Add your module protected function declarations here.
#pragma endregion

private:
	UPROPERTY()
	UMatchmakingP2POnlineSession_Starter* MatchmakingOnlineSession;
};
