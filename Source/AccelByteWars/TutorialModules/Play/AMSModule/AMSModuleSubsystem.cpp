// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if UE_EDITOR || UE_SERVER

#include "AMSModuleSubsystem.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "AMSModuleLog.h"

void UAMSModuleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG_AMS_MODULE(Log, TEXT("AMS Module subsystem initialized."));

	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.AddUObject(this, &ThisClass::UnregisterServer);
}

void UAMSModuleSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG_AMS_MODULE(Log, TEXT("AMS Module subsystem deinitialized."));

	AAccelByteWarsGameSession::OnRegisterServerDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.RemoveAll(this);
}

void UAMSModuleSubsystem::RegisterServer(const FName SessionName)
{
	if (bIsRegistering)
	{
		UE_LOG_AMS_MODULE(Warning, TEXT("Cannot register server. Register server is already in progress."));
		return;
	}

	if (AccelByte::FRegistry::ServerAMS.IsConnected()) 
	{
		UE_LOG_AMS_MODULE(Warning, TEXT("Server is already registered and connected to AMS websocket."));
		return;
	}

	UE_LOG_AMS_MODULE(Log, TEXT("Register server to AMS."));

	bIsRegistering = true;
	CurrentSessionName = SessionName;

	ConnectionTimeOutTimer = ConnectionTimeOut;
	DisconnectionTimeOutTimer = DisconnectionTimeOut;

	Connect();
}

void UAMSModuleSubsystem::UnregisterServer(const FName SessionNam)
{
	UE_LOG_AMS_MODULE(Log, TEXT("Unregister server from AMS."));

	CurrentSessionName = FName(FString());
	Disconnect();
}

void UAMSModuleSubsystem::Connect()
{
	if (AccelByte::FRegistry::ServerAMS.IsConnected())
	{
		UE_LOG_AMS_MODULE(Log, TEXT("Already connected to AMS websocket."));
		OnConnectSuccess();
		return;
	}

	OnConnectSuccessDelegate = AccelByte::GameServerApi::ServerAMS::FConnectSuccess::CreateUObject(this, &ThisClass::OnConnectSuccess);
	OnConnectErrorDelegate = AccelByte::GameServerApi::ServerAMS::FConnectError::CreateUObject(this, &ThisClass::OnConnectError);
	OnConnectClosedDelegate = AccelByte::GameServerApi::ServerAMS::FConnectionClosed::CreateUObject(this, &ThisClass::OnConnectClosed);
	AccelByte::FRegistry::ServerAMS.SetOnConnectSuccess(OnConnectSuccessDelegate);
	AccelByte::FRegistry::ServerAMS.SetOnConnectError(OnConnectErrorDelegate);
	AccelByte::FRegistry::ServerAMS.SetOnConnectionClosed(OnConnectClosedDelegate);

	OnDrainReceivedDelegate = AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived::CreateUObject(this, &ThisClass::OnDrainReceived);
	AccelByte::FRegistry::ServerAMS.SetOnAMSDrainReceivedDelegate(OnDrainReceivedDelegate);

	// If bServerUseAMS is true, the connection to AMS websocket is already started by SDK automatically. Else, try to connect manually.
	GetWorld()->GetTimerManager().SetTimer(ConnectionTimerHandle, this, &ThisClass::CheckConnection, TimerRate, true);
	if (!AccelByte::FRegistry::ServerSettings.bServerUseAMS)
	{
		UE_LOG_AMS_MODULE(Log, TEXT("Connecting to AMS websocket."));
		AccelByte::FRegistry::ServerAMS.Connect();
	}
}

void UAMSModuleSubsystem::Disconnect()
{
	if (!AccelByte::FRegistry::ServerAMS.IsConnected())
	{
		UE_LOG_AMS_MODULE(Warning, TEXT("Cannot disconnect AMS websocket. The AMS websocket connection is not established."));
		return;
	}

	UnbindAllDelegates();

	UE_LOG_AMS_MODULE(Log, TEXT("Disconnecting from AMS websocket."));
	GetWorld()->GetTimerManager().SetTimer(DisconnectionTimerHandle, this, &ThisClass::CheckDisconnection, TimerRate, true);
	AccelByte::FRegistry::ServerAMS.Disconnect();
}

void UAMSModuleSubsystem::SendServerReady()
{
	if (!AccelByte::FRegistry::ServerAMS.IsConnected())
	{
		UE_LOG_AMS_MODULE(Warning, TEXT("Cannot set server ready. The AMS websocket connection is not established."));
		return;
	}

	UE_LOG_AMS_MODULE(Log, TEXT("Send server ready message to AMS."));
	AccelByte::FRegistry::ServerAMS.SendReadyMessage();
}

void UAMSModuleSubsystem::OnConnectSuccess()
{
	UE_LOG_AMS_MODULE(Log, TEXT("Success to connect to AMS websocket."));
	UnbindConnectDelegates();

	/* It is not required to set the server as ready immediately after the AMS websocket connection is established.
	 * If the server needs to perform setup tasks before welcoming the player, the server ready message should be sent afterward.
	 * Since Byte Wars does not require such setup, the server ready message is sent immediately here. */
	SendServerReady();
}

void UAMSModuleSubsystem::OnConnectError(const FString& ErrorMessage)
{
	UE_LOG_AMS_MODULE(Warning, TEXT("Failed to connect to AMS websocket. Error: %s"), *ErrorMessage);
	CurrentSessionName = FName(FString());
	UnbindConnectDelegates();
}

void UAMSModuleSubsystem::OnConnectClosed(int32 StatusCode, FString const& Reason, bool bWasClean)
{
	UE_LOG_AMS_MODULE(Warning, TEXT("Failed to connect to AMS websocket. Connection is closed. Reason: %s"), *Reason);
	CurrentSessionName = FName(FString());
	UnbindConnectDelegates();
}

void UAMSModuleSubsystem::OnDisconnected(bool bIsSucceeded, const FString& ErrorMessage)
{
	UE_LOG_AMS_MODULE(Log, TEXT("Disconnected from AMS websocket. Success: %s. Error: %s"), bIsSucceeded ? TEXT("TRUE") : TEXT("FALSE"), *ErrorMessage);
	FPlatformMisc::RequestExit(false);
}

void UAMSModuleSubsystem::OnDrainReceived()
{
	UE_LOG_AMS_MODULE(Log, TEXT("Drain event received."));

	/**
	 * When a drain event occurs, the server may perform some clean-up tasks.
	 * Drain behavior on Byte Wars:
	 * When the server is currently waiting for players, unregisters and shuts down.
	 * Otherwise, keep it running as normal.
	 */
	if (const AAccelByteWarsMainMenuGameState* ABMainMenuGameState = Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()))
	{
		if (ABMainMenuGameState->LobbyStatus == ELobbyStatus::IDLE)
		{
			UE_LOG_AMS_MODULE(Log, TEXT("Game is currently waiting for players, shutting down."));
			UnregisterServer(CurrentSessionName);
			return;
		}
	}

	UE_LOG_AMS_MODULE(Log, TEXT("Game is currently in progress, drain event ignored."));
}

void UAMSModuleSubsystem::CheckConnection()
{
	if (AccelByte::FRegistry::ServerAMS.IsConnected()) 
	{
		ConnectionTimeOutTimer = ConnectionTimeOut;
		GetWorld()->GetTimerManager().ClearTimer(ConnectionTimerHandle);
		return;
	}

	// On time-out, retry to connect.
	if (ConnectionTimeOutTimer <= 0)
	{
		ConnectionTimeOutTimer = ConnectionTimeOut;
		GetWorld()->GetTimerManager().ClearTimer(ConnectionTimerHandle);
		UnbindConnectDelegates();

		if (ConnectionRetry > 0) 
		{
			ConnectionRetry--;
			Connect();
		}
		else 
		{
			OnConnectError(TEXT("Failed to connect to AMS websocket. Connection time out."));
		}
		return;
	}

	ConnectionTimeOutTimer--;
}

void UAMSModuleSubsystem::CheckDisconnection()
{
	if (!AccelByte::FRegistry::ServerAMS.IsConnected())
	{
		DisconnectionTimeOutTimer = DisconnectionTimeOut;
		GetWorld()->GetTimerManager().ClearTimer(DisconnectionTimerHandle);
		OnDisconnected(true, TEXT(""));
		return;
	}

	// On time-out, force to disconnect abruptly. 
	if (DisconnectionTimeOutTimer <= 0)
	{
		DisconnectionTimeOutTimer = DisconnectionTimeOut;
		GetWorld()->GetTimerManager().ClearTimer(DisconnectionTimerHandle);
		OnDisconnected(false, TEXT("Failed to disconnect from AMS websocket. Connection time out."));
		return;
	}

	DisconnectionTimeOutTimer--;
}

void UAMSModuleSubsystem::UnbindConnectDelegates()
{
	bIsRegistering = false;

	OnConnectSuccessDelegate.Unbind();
	OnConnectErrorDelegate.Unbind();
	OnConnectClosedDelegate.Unbind();
}

void UAMSModuleSubsystem::UnbindAllDelegates()
{
	UnbindConnectDelegates();
	OnDrainReceivedDelegate.Unbind();
}

#endif