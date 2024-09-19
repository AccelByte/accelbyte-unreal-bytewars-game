// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystemAMS.h"

#include "MultiplayerDSEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-2", "9-10", "14"]}
// @@@MULTISNIP BindRegisterServerDelegate {"selectedLines": ["1-2", "5", "14"]}
// @@@MULTISNIP BindSendServerReadyDelegate {"selectedLines": ["1-2", "6", "14"]}
// @@@MULTISNIP BindUnregisterServerDelegate {"selectedLines": ["1-2", "7", "14"]}
// @@@MULTISNIP BindAMSDrainDelegate {"selectedLines": ["1-2", "12", "14"]}
// @@@MULTISNIP BindSessionEndDelegate {"selectedLines": ["1-2", "13", "14"]}
void UMultiplayerDSEssentialsSubsystemAMS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::SendServerReady);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.AddUObject(this, &ThisClass::UnregisterServer);

	ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface());
	ensure(ABSessionInt);
	
	ABSessionInt->OnAMSDrainReceivedDelegates.AddUObject(this, &ThisClass::OnAMSDrainReceived);
	ABSessionInt->OnV2SessionEndedDelegates.AddUObject(this, &ThisClass::OnV2SessionEnded);
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-Deinitialize
// @@@MULTISNIP UnbindRegisterServerDelegate {"selectedLines": ["1-2", "5", "10"]}
// @@@MULTISNIP UnbindSendServerReadyDelegate {"selectedLines": ["1-2", "5", "10"]}
// @@@MULTISNIP UnbindUnregisterServerDelegate {"selectedLines": ["1-2", "6", "10"]}
// @@@MULTISNIP UnbindAMSDrainDelegate {"selectedLines": ["1-2", "8", "10"]}
// @@@MULTISNIP UnbindSessionEndDelegate {"selectedLines": ["1-2", "9-10"]}
void UMultiplayerDSEssentialsSubsystemAMS::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameSession::OnRegisterServerDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.RemoveAll(this);

	ABSessionInt->OnAMSDrainReceivedDelegates.RemoveAll(this);
	ABSessionInt->OnV2SessionEndedDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-RegisterServer
void UMultiplayerDSEssentialsSubsystemAMS::RegisterServer(const FName SessionName)
{
	UE_LOG_MultiplayerDSEssentials(Verbose, TEXT("called"))

	// safety
	if (!ABSessionInt)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Session interface null"))
		OnRegisterServerComplete(false);
		return;
	}
	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Is not DS"));
		OnRegisterServerComplete(false);
		return;
	}

	if (bServerAlreadyRegister)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Already registered"));
		OnRegisterServerComplete(false);
		return;
	}

	ABSessionInt->RegisterServer(SessionName, FOnRegisterServerComplete::CreateUObject(
		this, &ThisClass::OnRegisterServerComplete));
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-OnRegisterServerComplete
void UMultiplayerDSEssentialsSubsystemAMS::OnRegisterServerComplete(const bool bSucceeded)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		bServerAlreadyRegister = true;
	}

	AAccelByteWarsGameMode::OnRegisterServerCompleteDelegates.Broadcast(bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-UnregisterServer
void UMultiplayerDSEssentialsSubsystemAMS::UnregisterServer(const FName SessionName)
{
	UE_LOG_MultiplayerDSEssentials(Verbose, TEXT("called"))

	// safety
	if (!ABSessionInt)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Session interface null"))
		OnUnregisterServerComplete(false);
		return;
	}
	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Is not DS"));
		OnUnregisterServerComplete(false);
		return;
	}

	ABSessionInt->UnregisterServer(SessionName, FOnUnregisterServerComplete::CreateUObject(
		this, &ThisClass::OnUnregisterServerComplete));
	bUnregisterServerRunning = true;
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-OnUnregisterServerComplete
void UMultiplayerDSEssentialsSubsystemAMS::OnUnregisterServerComplete(const bool bSucceeded)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bUnregisterServerRunning = false;

	FPlatformMisc::RequestExit(false);
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-SendServerReady
void UMultiplayerDSEssentialsSubsystemAMS::SendServerReady(const FName SessionName)
{
	if (!ABSessionInt)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Session interface null"));
		OnSendServerReadyComplete(false);
		return;
	}

	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Is not DS"));
		OnSendServerReadyComplete(false);
		return;
	}

	if (bServerAlreadyRegister)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Already registered and ready"));
		OnSendServerReadyComplete(false);
		return;
	}

	// Registering the server manually by setting it as ready.
	ABSessionInt->SendServerReady(SessionName, FOnRegisterServerComplete::CreateUObject(this, &ThisClass::OnSendServerReadyComplete));
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-OnSendServerReadyComplete
void UMultiplayerDSEssentialsSubsystemAMS::OnSendServerReadyComplete(const bool bSucceeded)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	if (bSucceeded) 
	{
		bServerAlreadyRegister = true;
	}
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-OnAMSDrainReceived
void UMultiplayerDSEssentialsSubsystemAMS::OnAMSDrainReceived()
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("Received AMS drain message; Shutting down the server now!"));

	OnUnregisterServerComplete(true);
}
// @@@SNIPEND

// @@@SNIPSTART MultiplayerDSEssentialsSubsystemAMS.cpp-OnV2SessionEnded
void UMultiplayerDSEssentialsSubsystemAMS::OnV2SessionEnded(const FName SessionName)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("Received AMS session ended notification; Shutting down the server now!"));

	UnregisterServer(SessionName);
}
// @@@SNIPEND
