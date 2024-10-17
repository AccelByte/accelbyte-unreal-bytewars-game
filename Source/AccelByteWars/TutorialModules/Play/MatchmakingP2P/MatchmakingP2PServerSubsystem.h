// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchmakingP2POnlineSession.h"
#include "Play/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchmakingP2PServerSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingP2PServerSubsystem : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

// @@@SNIPSTART MatchmakingP2PServerSubsystem.h-protected
// @@@MULTISNIP OnServerSessionReceived {"selectedLines": ["1-2"]}
protected:
	virtual void OnServerSessionReceived(FName SessionName) override;
// @@@SNIPEND

private:
	UPROPERTY()
	UMatchmakingP2POnlineSession* MatchmakingOnlineSession;
};
