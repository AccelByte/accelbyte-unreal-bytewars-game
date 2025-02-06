// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if UE_EDITOR || UE_SERVER

#include "AMSModuleSubsystem_Starter.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "AMSModuleLog.h"

void UAMSModuleSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG_AMS_MODULE(Log, TEXT("AMS Module subsystem initialized."));

	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.AddUObject(this, &ThisClass::UnregisterServer);
}

void UAMSModuleSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG_AMS_MODULE(Log, TEXT("AMS Module subsystem deinitialized."));

	AAccelByteWarsGameSession::OnRegisterServerDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.RemoveAll(this);
}

void UAMSModuleSubsystem_Starter::RegisterServer(const FName SessionName)
{
#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion 
}

void UAMSModuleSubsystem_Starter::UnregisterServer(const FName SessionNam)
{
#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion 
}

#pragma region "Tutorial"
// Place your implementation here
#pragma endregion 

void UAMSModuleSubsystem_Starter::UnbindConnectDelegates()
{
	bIsRegistering = false;

	OnConnectSuccessDelegate.Unbind();
	OnConnectErrorDelegate.Unbind();
	OnConnectClosedDelegate.Unbind();
}

void UAMSModuleSubsystem_Starter::UnbindAllDelegates()
{
	UnbindConnectDelegates();
	OnDrainReceivedDelegate.Unbind();
}

#endif