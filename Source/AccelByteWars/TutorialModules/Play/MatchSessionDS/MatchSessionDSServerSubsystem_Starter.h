// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchSessionDSOnlineSession_Starter.h"
#include "Play/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchSessionDSServerSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionDSServerSubsystem_Starter : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

#pragma region "Game specific"
protected:
	virtual void OnAuthenticatePlayerComplete_PrePlayerSetup(APlayerController* PlayerController) override;
#pragma endregion 

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	UPROPERTY()
	UMatchSessionDSOnlineSession_Starter* OnlineSession;

#pragma region "Match Session with DS Function Declarations"
protected:
	// TODO: Add your module protected function declarations here.
#pragma endregion 
};
