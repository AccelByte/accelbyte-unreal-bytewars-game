// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchmakingDSOnlineSession.h"
#include "Play/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchmakingDSServerSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchmakingDSServerSubsystem : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	virtual void OnServerSessionReceived(FName SessionName) override;

private:
	UPROPERTY()
	UMatchmakingDSOnlineSession* MatchmakingOnlineSession;
};
