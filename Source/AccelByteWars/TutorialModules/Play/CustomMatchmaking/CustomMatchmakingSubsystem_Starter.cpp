// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingSubsystem_Starter.h"

#include "CustomMatchmakingLog.h"
#include "JsonObjectConverter.h"
#include "WebSocketsModule.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

void UCustomMatchmakingSubsystem_Starter::CleanupWebSocket()
{
#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion
}

void UCustomMatchmakingSubsystem_Starter::SetupWebSocket()
{
	// Get URL from Launch param / config file
	FString ServerUrl = DEFAULT_MATCHMAKING_CONFIG_URL;
	bool bFound = false;

	// Check launch param
	const FString CmdArgs = FCommandLine::Get();
	const FString CmdKeyword = FString::Printf(TEXT("-%s="), *CUSTOM_MATCHMAKING_CONFIG_KEY_URL);
	if (CmdArgs.Contains(*CmdKeyword, ESearchCase::IgnoreCase))
	{
		FParse::Value(*CmdArgs, *CmdKeyword, ServerUrl);
		bFound = true;
	}

	// Check DefaultEngine.ini file
	if (!bFound)
	{
		GConfig->GetString(*CUSTOM_MATCHMAKING_CONFIG_SECTION_URL, *CUSTOM_MATCHMAKING_CONFIG_KEY_URL, ServerUrl, GEngineIni);
	}

	// WebSocket setup
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("WebSocket target URL: %s"), *ServerUrl)

#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion
}

void UCustomMatchmakingSubsystem_Starter::ThrowInvalidPayloadError()
{
	UE_LOG_CUSTOMMATCHMAKING(Warning, TEXT("Unexpected message format was received from the Matchmaking service, closing websocket"))
	PendingDisconnectReason = TEXT_WEBSOCKET_PARSE_ERROR;
	WebSocket->Close(WEBSOCKET_ERROR_CODE_UNEXPECTED_MESSAGE, TEXT_WEBSOCKET_PARSE_ERROR);
}

#pragma region "Tutorial"
// Place your implementation here
#pragma endregion
