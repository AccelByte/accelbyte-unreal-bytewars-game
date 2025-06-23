// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "GameTelemetrySubsystem_Starter.h"

#include "GameTelemetryLog.h"
#include "GameTelemetryModels.h"
#include "OnlineGameStandardEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"

void UGameTelemetrySubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Game Standard Event interface
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(OnlineSubsystem))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "OnlineSubystem is null. Initialization canceled.")
		return;
	}

	const FOnlineSubsystemAccelByte* AccelByteOnlineSubsystem = static_cast<FOnlineSubsystemAccelByte*>(OnlineSubsystem);
	if (!ensure(AccelByteOnlineSubsystem))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "AccelByteOnlineSubystem is null. Initialization canceled.")
		return;
	}

	GameStandardEventInterface = AccelByteOnlineSubsystem->GetGameStandardEventInterface();
	if (!ensure(GameStandardEventInterface))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "GameStandardEvent interface is null. Initialization canceled.")
		return;
	}

	// Bind to game's delegates.
	AAccelByteWarsInGameGameMode::OnGameStartedDelegates.AddUObject(this, &ThisClass::OnGameStarted);
	AAccelByteWarsInGameGameMode::OnGameEndsDelegate.AddUObject(this, &ThisClass::OnGameEnded);
	AAccelByteWarsInGameGameMode::OnPlayerEnteredMatch.AddUObject(this, &ThisClass::OnPlayerEnteredMatch);
	AAccelByteWarsInGameGameMode::OnEntityDestroyedDelegates.AddUObject(this, &ThisClass::OnEntityDestroyed);
}

void UGameTelemetrySubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// Unbind from game's delegates
	AAccelByteWarsInGameGameMode::OnGameStartedDelegates.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnGameEndsDelegate.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnPlayerEnteredMatch.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnEntityDestroyedDelegates.RemoveAll(this);
}

void UGameTelemetrySubsystem_Starter::OnGameStarted()
{
	// put your code here
}

void UGameTelemetrySubsystem_Starter::OnGameEnded(const FString& Reason)
{
	// put your code here
}

void UGameTelemetrySubsystem_Starter::OnPlayerEnteredMatch(const FUniqueNetIdPtr PlayerNetId) const
{
	// put your code here
}

void UGameTelemetrySubsystem_Starter::OnEntityDestroyed(
	const FString& DestroyedEntityType,
	const FUniqueNetIdPtr DestroyedPlayerId,
	const FString& DestroyedEntityId,
	const FVector& DestroyedLocation,
	const FString& SourceEntityType,
	const FString& SourceEntityId) const
{
	// put your code here
}

TSharedPtr<FNamedOnlineSession> UGameTelemetrySubsystem_Starter::GetGameOnlineSession() const
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(OnlineSubsystem))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Online subsystem is invalid.")
		return nullptr;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!ensure(SessionInterface))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Online session interface is invalid.")
		return nullptr;
	}

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!OnlineSession)
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Online session is invalid.")
		return nullptr;
	}

	// Need to specify deleter to avoid crashes due to the default deleter deletes pointer inside of OnlineSession once this specific ptr is destroyed. 
	return MakeShareable(OnlineSession, [](FNamedOnlineSession*){return;});
}

FString UGameTelemetrySubsystem_Starter::GetFormattedGameMode(TSharedPtr<FNamedOnlineSession> NamedOnlineSession) const
{
	FString FormattedGameMode = TEXT("");

	// Only run if current user is a host or DS.
	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	if (!GameStateBase || !GameStateBase->HasAuthority())
	{
		// Might happen if the local user is not a host or server.
		UE_LOG_GAME_TELEMETRY(Log, "Local user is not a host or server, canceled.")
		return FormattedGameMode;
	}

	// Get game mode from game setup in GameState.
	const AAccelByteWarsGameState* ByteWarsGameState = Cast<AAccelByteWarsGameState>(GameStateBase);
	if (!ensure(ByteWarsGameState))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Current GameState is not AAccelByteWarsGameState, canceled.")
		return FormattedGameMode;
	}

	FString GameModeTypeString = TEXT("");
	switch (ByteWarsGameState->GameSetup.GameModeType)
	{
	case EGameModeType::FFA:
		GameModeTypeString = TEXT_GAME_MODE_FFA;
		break;
	case EGameModeType::TDM:
		GameModeTypeString = TEXT_GAME_MODE_TDM;
		break;
	}

	// Get session type from session info.
	// If this is an offline play, the online session will be empty.
	FString SessionTypeString = TEXT_SESSION_TYPE_LOCAL;
	if (NamedOnlineSession.IsValid())
	{
		const FOnlineSessionSettings& SessionSettings = NamedOnlineSession->SessionSettings;

		bool bIsCustomGame = false;
		SessionSettings.Get(GAMESETUP_IsCustomGame, bIsCustomGame);
		if (bIsCustomGame)
		{
			// By design of Byte Wars session integration, if is custom game is set to true, it is always a custom match. Don't need to check other setting. 
			SessionTypeString = TEXT_SESSION_TYPE_CUSTOMMATCH;
		}
		else
		{
			FString SessionRequestType = TEXT("");
			SessionSettings.Get(GAME_SESSION_REQUEST_TYPE, SessionRequestType);

			// By design of Byte Wars session integration, if game session request setting is not set, it is matchmaking.
			SessionTypeString = SessionRequestType.Equals(
				GAME_SESSION_REQUEST_TYPE_MATCHSESSION) ? TEXT_SESSION_TYPE_MATCHSESSION : TEXT_SESSION_TYPE_MATCHMAKING;
		}
	}

	// Format string.
	FormattedGameMode = FString::Printf(TEXT("%s:%s"), *SessionTypeString, *GameModeTypeString);
	return FormattedGameMode;
}

int32 UGameTelemetrySubsystem_Starter::GetPlayerTeamId(const FUniqueNetIdPtr PlayerNetId) const
{
	// Only run if current user is a host or DS.
	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	if (!GameStateBase || !GameStateBase->HasAuthority())
	{
		// Might happen if the local user is not a host or server.
		UE_LOG_GAME_TELEMETRY(Log, "Local user is not a host or server, canceled.")
		return INDEX_NONE;
	}

	const AAccelByteWarsGameState* ByteWarsGameState = Cast<AAccelByteWarsGameState>(GameStateBase);
	if (!ensure(ByteWarsGameState))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Current GameState is not AAccelByteWarsGameState, canceled.")
		return INDEX_NONE;
	}

	// Get player data.
	for (const FGameplayTeamData& GameplayTeamData : ByteWarsGameState->Teams)
	{
		for (const FGameplayPlayerData& GameplayPlayerData : GameplayTeamData.TeamMembers)
		{
			if (CompareAccelByteUniqueId(PlayerNetId, GameplayPlayerData.UniqueNetId.GetUniqueNetId()))
			{
				return GameplayTeamData.TeamId;
			}
		}
	}

	UE_LOG_GAME_TELEMETRY(Warning, "Can't find player with specifed NetId, canceled.")
	return INDEX_NONE;
}

bool UGameTelemetrySubsystem_Starter::CompareAccelByteUniqueId(
	const FUniqueNetIdPtr FirstUniqueNetId,
	const FUniqueNetIdPtr SecondUniqueNetId) const
{
	if (!FirstUniqueNetId.IsValid() || !SecondUniqueNetId.IsValid())
	{
		return false;
	}

	// compare directly first.
	if (FirstUniqueNetId == SecondUniqueNetId)
	{
		return true;
	}

	// if false, attempt to compare AB User Id first.
	if (!FirstUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE) ||
		!SecondUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
	{
		return false;
	}
	
	const FUniqueNetIdAccelByteUserPtr FirstAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*FirstUniqueNetId);
	const FUniqueNetIdAccelByteUserPtr SecondAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*SecondUniqueNetId);

	if (!FirstAbUniqueNetId.IsValid() || !SecondAbUniqueNetId.IsValid())
	{
		return false;
	}

	const FString FirstAbUserId = FirstAbUniqueNetId->GetAccelByteId();
	const FString SecondAbUserId = SecondAbUniqueNetId->GetAccelByteId();

	return FirstAbUserId.Equals(SecondAbUserId);
}
