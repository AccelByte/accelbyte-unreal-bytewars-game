// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "MatchSessionDSOnlineSession.h"
#include "TutorialModules/GameSessionEssentials/AccelByteWarsServerSubsystemBase.h"
#include "MatchSessionDSServerSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UMatchSessionDSServerSubsystem : public UAccelByteWarsServerSubsystemBase
{
	GENERATED_BODY()

#pragma region "Game specific"
protected:
	virtual void OnAuthenticatePlayerComplete_PrePlayerSetup(APlayerController* PlayerController) override;
#pragma endregion 

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual TSubclassOf<UTutorialModuleOnlineSession> GetOnlineSessionClass() const override
	{
		return UMatchSessionDSOnlineSession::StaticClass();
	}

	virtual void RegisterServer(const FName SessionName) override;
	virtual void UnregisterServer(const FName SessionName) override;

protected:
	virtual void OnRegisterServerComplete(bool bSucceeded) override;
	virtual void OnUnregisterServerComplete(bool bSucceeded) override;

	virtual void OnServerSessionReceived(FName SessionName) override;

private:
	bool bServerAlreadyRegister = false;
	bool bUnregisterServerRunning = false;

	UPROPERTY()
	UMatchSessionDSOnlineSession* OnlineSession;
};
