// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/System/AccelByteWarsGameSession.h"

void AAccelByteWarsGameSession::RegisterServer()
{
	Super::RegisterServer();

	if (OnRegisterServerDelegate.IsBound())
	{
		OnRegisterServerDelegate.Broadcast(SessionName);
	}
}

void AAccelByteWarsGameSession::UnregisterServer()
{
	// Gracefully unregister and shut down server. Used for AMS DS.
	if (OnUnregisterServerDelegate.IsBound()) 
	{
		OnUnregisterServerDelegate.Broadcast(SessionName);
	}
	// Shut down server immediately without unregistering the server. Used for non-AMS DS.
	else 
	{
		FPlatformMisc::RequestExit(false);
	}
}