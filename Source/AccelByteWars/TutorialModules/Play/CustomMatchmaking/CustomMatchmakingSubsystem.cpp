// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingSubsystem.h"

#include "CustomMatchmakingLog.h"
#include "JsonObjectConverter.h"
#include "SocketSubsystem.h"
#include "WebSocketsModule.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

void UCustomMatchmakingSubsystem::StartMatchmaking()
{
	// In this sample, connecting to the websocket will immediately start the matchmaking
	SetupWebSocket();
	WebSocket->Connect();
}

void UCustomMatchmakingSubsystem::StopMatchmaking()
{
	// Cancel the matchmaking by disconnecting from the websocket
	if (!WebSocket)
	{
		UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket is null, operation cancelled"))
		return;
	}

	PendingDisconnectReason = TEXT_ERROR_CANCELED;
	WebSocket->Close();
}

void UCustomMatchmakingSubsystem::CleanupWebSocket()
{
	PendingDisconnectReason = TEXT("");

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

	// Reset stored disconnect reason
	PendingDisconnectReason = TEXT("");

	// WebSocket setup
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("WebSocket target URL: %s"), *ServerUrl)
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl, WEBSOCKET_PROTOCOL);

	// Bind events
	WebSocket->OnConnected().AddUObject(this, &ThisClass::OnConnected);
	WebSocket->OnConnectionError().AddUObject(this, &ThisClass::OnError);
	WebSocket->OnClosed().AddUObject(this, &ThisClass::OnClosed);
	WebSocket->OnMessage().AddUObject(this, &ThisClass::OnMessage);
}

void UCustomMatchmakingSubsystem::OnConnected() const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket connected"))
	OnMatchmakingStartedDelegates.Broadcast();
}

void UCustomMatchmakingSubsystem::OnClosed(int32 StatusCode, const FString& Reason, bool WasClean)
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket closed: (%d) %s"), StatusCode, *Reason)

	// Modify the error message to make it more user-friendly
	FString ModifiedReason = Reason;
	if (!PendingDisconnectReason.IsEmpty())
	{
		ModifiedReason = PendingDisconnectReason;
	}

	OnMatchmakingStoppedDelegates.Broadcast(ModifiedReason);

	CleanupWebSocket();
}

void UCustomMatchmakingSubsystem::OnMessage(const FString& Message)
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket message received: %s"), *Message)
	if (Message.IsEmpty())
	{
		UE_LOG_CUSTOMMATCHMAKING(VeryVerbose, TEXT("Message is empty, skipping"))
		return;
	}

	// Parse message
	FMatchmakerPayload Payload;
	// Safety in case the received message is different than expected
	if (!FJsonObjectConverter::JsonObjectStringToUStruct<FMatchmakerPayload>(Message, &Payload))
	{
		ThrowInvalidPayloadError();
		return;
	}

	// Notify
	OnMatchmakingMessageReceivedDelegates.Broadcast(Payload);

	// Travel if server is ready
	if (Payload.Type == EMatchmakerPayloadType::OnServerReady)
	{
		// Use FInternetAddr to check whether this is a valid IPv4 or IPv6
		const TSharedPtr<FInternetAddr> ServerAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool bIsValid = false;

		ServerAddress->SetIp(*Payload.Message, bIsValid);
		if (!bIsValid)
		{
			ThrowInvalidPayloadError();
			return;
		}

		// Trigger travel
		AAccelByteWarsPlayerController* PC = Cast<AAccelByteWarsPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!PC)
		{
			UE_LOG_CUSTOMMATCHMAKING(Warning, TEXT("Player Controller is not AAccelByteWarsPlayerController, cancelling travel"))
			return;
		}

		const FString Address = ServerAddress->ToString(true);
		PC->DelayedClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void UCustomMatchmakingSubsystem::OnError(const FString& Error) const
{
	UE_LOG_CUSTOMMATCHMAKING(Verbose, TEXT("Websocket error: %s"), *Error)

	// Modify the error message to make it more user-friendly
	FString ModifiedReason = Error;
	if (ModifiedReason.Contains(WEBSOCKET_FAILED_GENERIC_MESSAGE, ESearchCase::IgnoreCase) || ModifiedReason.IsEmpty())
	{
		ModifiedReason = TEXT_WEBSOCKET_ERROR_GENERIC;
	}

	OnMatchmakingErrorDelegates.Broadcast(ModifiedReason);
}

void UCustomMatchmakingSubsystem::ThrowInvalidPayloadError()
{
	UE_LOG_CUSTOMMATCHMAKING(Warning, TEXT("Unexpected message format was received from the Matchmaking service, closing websocket"))
	PendingDisconnectReason = TEXT_WEBSOCKET_PARSE_ERROR;
	WebSocket->Close(WEBSOCKET_ERROR_CODE_UNEXPECTED_MESSAGE, TEXT_WEBSOCKET_PARSE_ERROR);
}
