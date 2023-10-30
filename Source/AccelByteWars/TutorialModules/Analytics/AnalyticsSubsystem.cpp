// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Analytics/AnalyticsSubsystem.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteGameTelemetryApi.h"
#include "GameServerApi/AccelByteServerGameTelemetryApi.h"

#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

DEFINE_LOG_CATEGORY(LogAnalyticsEssentials);

void UAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Send player's death telemetry.
	AAccelByteWarsInGameGameMode::OnPlayerDieDelegate.AddWeakLambda(this, [this](const APlayerController* Player, const AActor* PlayerActor, const APlayerController* Killer)
	{
		if (!Player || !PlayerActor || !Killer) 
		{
			UE_LOG_ANALYTICS(Warning, TEXT("Cannot send player's death telemetry. Either the player or killer is not valid."));
			return;
		}

		const AAccelByteWarsPlayerState* PlayerState = Cast<AAccelByteWarsPlayerState>(Player->PlayerState);
		const AAccelByteWarsPlayerState* KillerState = Cast<AAccelByteWarsPlayerState>(Killer->PlayerState);
		if (!PlayerState || !KillerState) 
		{
			UE_LOG_ANALYTICS(Warning, TEXT("Cannot send player's death telemetry. Either the player or killer's PlayerState is not valid."));
			return;
		}

		// Collect player's death information.
		const bool bIsSuicide = (Player == Killer);
		const bool bIsFriendlyFire = (PlayerState->TeamId == KillerState->TeamId);
		const FString DeathCause = bIsSuicide ? FString("Suicide") : (bIsFriendlyFire ? FString("Killed by Teammate") : FString("Killed by Opponent"));
		const FVector2D DeathLocation(PlayerActor->GetActorLocation().X, PlayerActor->GetActorLocation().Y);

		// Construct player's death telemetry.
		TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
		Payload->SetStringField(FString("deathTimeStamp"), FDateTime::Now().ToIso8601());
		Payload->SetStringField(FString("deathType"), bIsFriendlyFire ? FString("Self") : FString("Opponent"));
		Payload->SetStringField(FString("deathCause"), DeathCause);
		Payload->SetStringField(FString("deathLocation"), FString::Printf(TEXT("%.3f,%.3f"), DeathLocation.X, DeathLocation.Y));
		Payload->SetNumberField(FString("attempsNeeded"), PlayerState->NumKilledAttemptInSingleLifetime);

		// Send player's death telemetry.
		SendTelemetry(FString("player_Died"), Payload, true);
	});
}

void UAnalyticsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	AAccelByteWarsInGameGameMode::OnPlayerDieDelegate.RemoveAll(this);

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
