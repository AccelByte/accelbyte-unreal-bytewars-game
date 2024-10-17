// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "SessionEssentialsOnlineSession_Starter.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "SessionEssentialsLog.h"

void USessionEssentialsOnlineSession_Starter::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// TODO: Register OSS delegates
}

void USessionEssentialsOnlineSession_Starter::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	// TODO: Unregister OSS delegates
}

FNamedOnlineSession* USessionEssentialsOnlineSession_Starter::GetSession(const FName SessionName)
{
	return GetSessionInt()->GetNamedSession(SessionName);
}

EAccelByteV2SessionType USessionEssentialsOnlineSession_Starter::GetSessionType(const FName SessionName)
{
	const FNamedOnlineSession* OnlineSession = GetSession(SessionName);
	if (!OnlineSession)
	{
		return EAccelByteV2SessionType::Unknown;
	}

	const FOnlineSessionSettings& OnlineSessionSettings = OnlineSession->SessionSettings;

	return GetABSessionInt()->GetSessionTypeFromSettings(OnlineSessionSettings);
}

FName USessionEssentialsOnlineSession_Starter::GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType)
{
	FName SessionName = FName();

	switch (SessionType)
	{
	case EAccelByteV2SessionType::GameSession:
		SessionName = GameSessionName;
		break;
	case EAccelByteV2SessionType::PartySession:
		SessionName = PartySessionName;
		break;
	default: ;
	}

	return SessionName;
}

#pragma region "Funtions implementation"
// TODO: implement your functions here
#pragma endregion 
