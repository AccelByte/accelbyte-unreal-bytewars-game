// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/System/AccelByteWarsGameSession.h"

void AAccelByteWarsGameSession::RegisterServer()
{
	Super::RegisterServer();

	if (OnRegisterServerDelegates.IsBound())
	{
		OnRegisterServerDelegates.Broadcast(SessionName);
	}
}

void AAccelByteWarsGameSession::UnregisterServer()
{
	// Used for AMS DS.
	if (OnUnregisterServerDelegates.IsBound()) 
	{
		OnUnregisterServerDelegates.Broadcast(SessionName);
	}
	// Shut down server immediately without unregistering the server. Used for non-AMS DS.
	else 
	{
		FPlatformMisc::RequestExit(false);
	}
}