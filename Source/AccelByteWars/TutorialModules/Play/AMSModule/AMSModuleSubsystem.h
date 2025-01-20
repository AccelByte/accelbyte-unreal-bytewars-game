// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#if UE_EDITOR || UE_SERVER
#include "GameServerApi/AccelByteServerAMSApi.h"
#endif
#include "AMSModuleSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UAMSModuleSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
#if UE_EDITOR || UE_SERVER
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterServer(const FName SessionName);
	void UnregisterServer(const FName SessionNam);

private:
	void Connect();
	void Disconnect();
	void SendServerReady();

	void OnConnectSuccess();
	void OnConnectError(const FString& ErrorMessage);
	void OnConnectClosed(int32 StatusCode, FString const& Reason, bool bWasClean);
	
	void OnDisconnected(bool bIsSucceeded, const FString& ErrorMessage);

	void OnDrainReceived();

	void CheckConnection();
	void CheckDisconnection();

	void UnbindConnectDelegates();
	void UnbindAllDelegates();

	AccelByte::GameServerApi::ServerAMS::FConnectSuccess OnConnectSuccessDelegate;
	AccelByte::GameServerApi::ServerAMS::FConnectError OnConnectErrorDelegate;
	AccelByte::GameServerApi::ServerAMS::FConnectionClosed OnConnectClosedDelegate;
	AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived OnDrainReceivedDelegate;

	FName CurrentSessionName;
	bool bIsRegistering = false;

	int32 ConnectionTimeOut = 30;
	int32 DisconnectionTimeOut = 30;

	float TimerRate = 1.0f;
	int32 ConnectionTimeOutTimer = 0;
	int32 DisconnectionTimeOutTimer = 0;

	int32 ConnectionRetry = 3;

	FTimerHandle ConnectionTimerHandle;
	FTimerHandle DisconnectionTimerHandle;
#endif
};
