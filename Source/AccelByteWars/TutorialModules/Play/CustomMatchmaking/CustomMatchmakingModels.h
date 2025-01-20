// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStopped)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingMessageReceived, const FString& /*Message*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingError, const FString& /*ErrorMessage*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingServerInfoReceived, const FString& /*Address*/)

#define CUSTOM_MATCHMAKING_CONFIG_SECTION_URL FString(TEXT("CustomMatchmaking"))
#define CUSTOM_MATCHMAKING_CONFIG_KEY_URL FString(TEXT("CustomMatchmakingUrl"))
#define DEFAULT_MATCHMAKING_CONFIG_URL TEXT("ws://127.0.0.1:8080")
#define	WEBSOCKET_PROTOCOL TEXT("ws")
#define TEXT_LOADING_TRAVELLING FString(TEXT("Travelling to server"))
#define TEXT_LOADING_FINDING_MATCH FString(TEXT("Finding Match"))
#define TEXT_LOADING_REQUEST FString(TEXT("Requesting"))
#define TEXT_LOADING_CANCEL FString(TEXT("Canceling"))
#define TEXT_ERROR_CANCELED FString(TEXT("Canceled"))
