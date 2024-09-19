// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Analytics/AnalyticsSubsystem.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteGameTelemetryApi.h"
#include "GameServerApi/AccelByteServerGameTelemetryApi.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

DEFINE_LOG_CATEGORY(LogAnalyticsEssentials);

void UAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Send player's death telemetry.
	AAccelByteWarsInGameGameState::OnPlayerDieDelegate.AddWeakLambda(this, [this](const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer)
	{
		if (!DeathPlayer || !Killer)
		{
			UE_LOG_ANALYTICS(Warning, TEXT("Cannot send player's death telemetry. Either the player or killer is not valid."));
			return;
		}

		if (!GetWorld() && GetWorld()->GetNetMode() == ENetMode::NM_Client)
		{
			UE_LOG_ANALYTICS(Log, TEXT("Game is running as client. Only standalone, P2P host, and dedicated server can send player's death telemetry."));
			return;
		}

		// Collect player's death information.
		const bool bIsSuicide = (DeathPlayer == Killer);
		const bool bIsFriendlyFire = (DeathPlayer->TeamId == Killer->TeamId);
		const FString DeathCause = bIsSuicide ? FString("Suicide") : (bIsFriendlyFire ? FString("Killed by Teammate") : FString("Killed by Opponent"));

		// Construct player's death telemetry.
		TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
		Payload->SetStringField(FString("deathTimeStamp"), FDateTime::Now().ToIso8601());
		Payload->SetStringField(FString("deathType"), bIsFriendlyFire ? FString("Self") : FString("Opponent"));
		Payload->SetStringField(FString("deathCause"), DeathCause);
		Payload->SetStringField(FString("deathLocation"), FString::Printf(TEXT("%.3f,%.3f"), DeathLocation.X, DeathLocation.Y));
		Payload->SetNumberField(FString("attempsNeeded"), DeathPlayer->NumKilledAttemptInSingleLifetime);

		// Send player's death telemetry.
		SendTelemetry(FString("player_Died"), Payload, true);
	});
}

void UAnalyticsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	AAccelByteWarsInGameGameState::OnPlayerDieDelegate.RemoveAll(this);

	// Clear game client telemetry immediate events.
	ClientImmediateTelemetryEventList.Empty();
	AccelByte::FRegistry::GameTelemetry.SetImmediateEventList(ClientImmediateTelemetryEventList);

	// Clear game server telemetry immediate events.
	ServerImmediateTelemetryEventList.Empty();
	AccelByte::FRegistry::GameTelemetry.SetImmediateEventList(ServerImmediateTelemetryEventList);
}

void UAnalyticsSubsystem::SendTelemetry(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent)
{
	const FString FixedEventName = FString::Printf(TEXT("unreal-%s"), *EventName);

	if (IsRunningDedicatedServer()) 
	{
		SendTelemetryServer(FixedEventName, Payload, bIsImmediateEvent);
	}
	else 
	{
		SendTelemetryClient(FixedEventName, Payload, bIsImmediateEvent);
	}
}

void UAnalyticsSubsystem::SendTelemetryClient(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent)
{
	UE_LOG_ANALYTICS(Log, TEXT("Sending telemetry (game client) with event name: %s"), *EventName);

	if (bIsImmediateEvent) 
	{
		ClientImmediateTelemetryEventList.AddUnique(EventName);
		AccelByte::FRegistry::GameTelemetry.SetImmediateEventList(ClientImmediateTelemetryEventList);
	}

	FAccelByteModelsTelemetryBody TelemetryBody;
	TelemetryBody.EventName = EventName;
	TelemetryBody.EventNamespace = TelemetryGameNamespace;
	TelemetryBody.Payload = Payload;

	AccelByte::FRegistry::GameTelemetry.Send(
		TelemetryBody, 
		AccelByte::FVoidHandler::CreateLambda([EventName]()
		{
			UE_LOG_ANALYTICS(Log, TEXT("Success to send telemetry (game client) with event name: %s"), *EventName);
		}), 
		FErrorHandler::CreateLambda([EventName](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_ANALYTICS(Warning, TEXT("Failed to send telemetry (game client) with event name: %s. Error %d: %s"), *EventName, ErrorCode, *ErrorMessage);
		}
	));
}

void UAnalyticsSubsystem::SendTelemetryServer(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent)
{
	UE_LOG_ANALYTICS(Log, TEXT("Sending telemetry (server) with event name: %s"), *EventName);

	if (bIsImmediateEvent)
	{
		ServerImmediateTelemetryEventList.AddUnique(EventName);
		AccelByte::FRegistry::ServerGameTelemetry.SetImmediateEventList(ServerImmediateTelemetryEventList);
	}

	FAccelByteModelsTelemetryBody TelemetryBody;
	TelemetryBody.EventName = EventName;
	TelemetryBody.EventNamespace = TelemetryGameNamespace;
	TelemetryBody.Payload = Payload;

	AccelByte::FRegistry::ServerGameTelemetry.Send(
		TelemetryBody,
		AccelByte::FVoidHandler::CreateLambda([EventName]()
		{
			UE_LOG_ANALYTICS(Log, TEXT("Success to send telemetry (server) with event name: %s"), *EventName);
		}),
		FErrorHandler::CreateLambda([EventName](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_ANALYTICS(Warning, TEXT("Failed to send telemetry (server) with event name: %s. Error %d: %s"), *EventName, ErrorCode, *ErrorMessage);
		}
	));
}
