// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchSessionP2POnlineSession.h"
#include "Play/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchSessionP2PServerSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionP2PServerSubsystem : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

// @@@SNIPSTART MatchSessionP2PServerSubsystem.h-protected
// @@@MULTISNIP OnAuthenticatePlayerComplete_PrePlayerSetup {"selectedLines": ["1", "3"]}
// @@@MULTISNIP OnServerSessionReceived {"selectedLines": ["1", "6"]}
protected:
#pragma region "Game specific"
	virtual void OnAuthenticatePlayerComplete_PrePlayerSetup(APlayerController* PlayerController) override;
#pragma endregion 

	virtual void OnServerSessionReceived(FName SessionName) override;
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2PServerSubsystem.h-private
// @@@MULTISNIP OnlineSession {"selectedLines": ["1-3"]}
private:
	UPROPERTY()
	UMatchSessionP2POnlineSession* OnlineSession;
// @@@SNIPEND
};
