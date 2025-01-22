// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingSubsystem.h"

#include "CustomMatchmakingLog.h"
#include "JsonObjectConverter.h"
#include "WebSocketsModule.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

void UCustomMatchmakingSubsystem::StartMatchmaking()
{
	// In this sample, connecting to the websocket will immediately start the matchmaking
	SetupWebSocket();
	WebSocket->Connect();
}

void UCustomMatchmakingSubsystem::StopMatchmaking() const
{
	// Cancel the matchmaking by disconnecting from the websocket
	if (!WebSocket)
	{
		UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket is null, operation cancelled"))
		return;
	}

	WebSocket->Close();
}

void UCustomMatchmakingSubsystem::CleanupWebSocket() const
{
	// Unbind events
	WebSocket->OnConnected().RemoveAll(this);
	WebSocket->OnConnectionError().RemoveAll(this);
	WebSocket->OnClosed().RemoveAll(this);
	WebSocket->OnMessage().RemoveAll(this);
}

void UCustomMatchmakingSubsystem::SetupWebSocket()
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
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl, WEBSOCKET_PROTOCOL);

	// Bind events
	WebSocket->OnConnected().AddUObject(this, &ThisClass::OnConnected);
	WebSocket->OnConnectionError().AddUObject(this, &ThisClass::OnError);
	WebSocket->OnClosed().AddUObject(this, &ThisClass::OnClosed);
	WebSocket->OnMessage().AddUObject(this, &ThisClass::OnMessage);
}

bool UCustomMatchmakingSubsystem::IsIpv4(const FString& Message) const
{
	const FString Pattern = TEXT(R"(^((25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)(:\d{1,5})?$)");
	const FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, Message);

	return Matcher.FindNext();
}

void UCustomMatchmakingSubsystem::OnConnected() const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket connected"))
	OnMatchmakingStartedDelegates.Broadcast();
}

void UCustomMatchmakingSubsystem::OnClosed(int32 StatusCode, const FString& Reason, bool WasClean) const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket closed: (%d) %s"), StatusCode, *Reason)
	OnMatchmakingStoppedDelegates.Broadcast(Reason);

	CleanupWebSocket();
}

void UCustomMatchmakingSubsystem::OnMessage(const FString& Message) const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket message received: %s"), *Message)
	if (Message.IsEmpty())
	{
		UE_LOG_CUSTOMMATCHMAKING(VeryVerbose, TEXT("Message is empty, skipping"))
		return;
	}

	// Parse message
	FMatchmakerPayload Payload;
	FJsonObjectConverter::JsonObjectStringToUStruct(Message, &Payload);

	// Notify
	OnMatchmakingMessageReceivedDelegates.Broadcast(Payload);

	// Travel if server is ready
	if (Payload.GetType() == EMatchmakerPayloadType::OnServerReady)
	{
		// Trigger travel
		AAccelByteWarsPlayerController* PC = Cast<AAccelByteWarsPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!PC)
		{
			UE_LOG_CUSTOMMATCHMAKING(Warning, TEXT("Player Controller is not AAccelByteWarsPlayerController, cancelling travel"))
			return;
		}

		PC->DelayedClientTravel(Payload.Message, ETravelType::TRAVEL_Absolute);
	}
}

void UCustomMatchmakingSubsystem::OnError(const FString& Error) const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket error: %s"), *Error)
	OnMatchmakingErrorDelegates.Broadcast(Error);
}
