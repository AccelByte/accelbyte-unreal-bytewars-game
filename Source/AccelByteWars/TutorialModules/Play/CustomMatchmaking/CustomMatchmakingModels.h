// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CustomMatchmakingModels.generated.h"

#define WEBSOCKET_ERROR_CODE_UNEXPECTED_MESSAGE (int) 4001

#define WEBSOCKET_FAILED_GENERIC_MESSAGE FString(TEXT("connect failed"))

#define CUSTOM_MATCHMAKING_CONFIG_SECTION_URL FString(TEXT("CustomMatchmaking"))
#define CUSTOM_MATCHMAKING_CONFIG_KEY_URL FString(TEXT("CustomMatchmakingUrl"))
#define DEFAULT_MATCHMAKING_CONFIG_URL TEXT("ws://127.0.0.1:8080")
#define	WEBSOCKET_PROTOCOL TEXT("ws")
#define TEXT_LOADING_TRAVELLING FString(TEXT("Travelling to server"))
#define TEXT_LOADING_FINDING_MATCH FString(TEXT("Finding Match"))
#define TEXT_LOADING_REQUEST FString(TEXT("Requesting"))
#define TEXT_LOADING_CANCEL FString(TEXT("Canceling"))
#define TEXT_ERROR_CANCELED FString(TEXT("Canceled"))
#define TEXT_WEBSOCKET_ERROR_GENERIC FString(TEXT("Connect failed.\nMake sure the Matchmaking server is running, reachable, and the address and port is set properly"))
#define TEXT_WEBSOCKET_PARSE_ERROR FString(TEXT("Received invalid payload format from Matchmaking server. Make sure you are running a compatible version."))

enum class EMatchmakerPayloadType : uint8
{
	OnFindingMatch,
	OnMatchFound,
	OnMatchError,
	OnServerReady
};

USTRUCT()
struct FMatchmakerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Message;

	EMatchmakerPayloadType GetType() const { return TypeMap[Type]; }

	bool IsValid() const
	{
		return !Type.IsEmpty();
	}

private:
	inline static TMap<FString, EMatchmakerPayloadType> TypeMap = {
		{"OnFindingMatch", EMatchmakerPayloadType::OnFindingMatch},
		{"OnMatchError", EMatchmakerPayloadType::OnMatchError},
		{"OnMatchFound", EMatchmakerPayloadType::OnMatchFound},
		{"OnServerReady", EMatchmakerPayloadType::OnServerReady}
	};
};

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStopped, const FString& /*Reason*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingMessageReceived, const FMatchmakerPayload& /*Payload*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingError, const FString& /*ErrorMessage*/)
