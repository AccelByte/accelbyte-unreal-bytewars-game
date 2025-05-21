// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystem.h"

#include "MultiplayerDSEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"

void UMultiplayerDSEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.AddUObject(this, &ThisClass::UnregisterServer);

	ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(GetWorld()));
	ensure(ABSessionInt);
}

void UMultiplayerDSEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameSession::OnRegisterServerDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.RemoveAll(this);
}

void UMultiplayerDSEssentialsSubsystem::RegisterServer(const FName SessionName)
{
	UE_LOG_MultiplayerDSEssentials(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!ABSessionInt)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Session interface is null"))
		OnRegisterServerComplete(false);
		return;
	}
	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("The game instance is not a dedicated server"));
		OnRegisterServerComplete(false);
		return;
	}

	if (bServerAlreadyRegister)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("The server is already registered"));
		OnRegisterServerComplete(false);
		return;
	}

	ABSessionInt->RegisterServer(SessionName, FOnRegisterServerComplete::CreateUObject(
		this, &ThisClass::OnRegisterServerComplete));
}

void UMultiplayerDSEssentialsSubsystem::OnRegisterServerComplete(const bool bSucceeded)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		bServerAlreadyRegister = true;
	}

	AAccelByteWarsGameMode::OnRegisterServerCompleteDelegates.Broadcast(bSucceeded);
}

void UMultiplayerDSEssentialsSubsystem::UnregisterServer(const FName SessionName)
{
	UE_LOG_MultiplayerDSEssentials(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!ABSessionInt)
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("Session interface is null"))
		OnUnregisterServerComplete(false);
		return;
	}
	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MultiplayerDSEssentials(Warning, TEXT("The game instance is not a dedicated server"));
		OnUnregisterServerComplete(false);
		return;
	}

	ABSessionInt->UnregisterServer(SessionName, FOnUnregisterServerComplete::CreateUObject(
		this, &ThisClass::OnUnregisterServerComplete));
	bUnregisterServerRunning = true;
}

void UMultiplayerDSEssentialsSubsystem::OnUnregisterServerComplete(const bool bSucceeded)
{
	UE_LOG_MultiplayerDSEssentials(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bUnregisterServerRunning = false;

	FPlatformMisc::RequestExit(false);
}
