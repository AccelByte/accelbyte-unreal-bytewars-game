// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "GameTelemetrySubsystem.h"

#include "GameTelemetryLog.h"
#include "GameTelemetryModels.h"
#include "OnlineGameStandardEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"

void UGameTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Game Standard Event interface.
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

void UGameTelemetrySubsystem::Deinitialize()
{
	Super::Deinitialize();

	// Unbind from game's delegates
	AAccelByteWarsInGameGameMode::OnGameStartedDelegates.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnGameEndsDelegate.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnPlayerEnteredMatch.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnEntityDestroyedDelegates.RemoveAll(this);
}

void UGameTelemetrySubsystem::OnGameStarted()
{
	// Generate info id.
	CurrentMatchInfoId = FMatchInfoId(FGuid::NewGuid().ToString());

	// Get Online Session.
	const TSharedPtr<FNamedOnlineSession> NamedOnlineSession = GetGameOnlineSession();
	if (!NamedOnlineSession)
	{
		UE_LOG_GAME_TELEMETRY(Log, "Online session is invalid. Sending event as offline game.")
	}

	// Construct game mode name.
	const FString FormattedGameModeString = GetFormattedGameMode(NamedOnlineSession);
	if (FormattedGameModeString.IsEmpty())
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Can't retrieve game mode. Please check log prior to this. Canceled.")
		return;
	}
	const FMatchGameMode MatchGameMode = FMatchGameMode(FormattedGameModeString);

	// Send event.
	const bool bResult = GameStandardEventInterface->SendMatchInfoEvent(
		LocalUserNum,
		CurrentMatchInfoId,
		NamedOnlineSession,
		MatchGameMode);
	UE_LOG_GAME_TELEMETRY(Log, "Sent succeeded: %s. Match Info ID: %s", *FString(bResult ? TEXT("TRUE") : TEXT("FALSE")), *CurrentMatchInfoId.ToString())
}

void UGameTelemetrySubsystem::OnGameEnded(const FString& Reason)
{
	// Only run if OnGameStarted has been executed.
	if (CurrentMatchInfoId.IsEmpty())
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Match info ID is empty. OnGameStarted might not been called, yet. Canceled.")
		return;
	}

	// Only run if current user is a host or DS.
	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	if (!GameStateBase || !GameStateBase->HasAuthority())
	{
		// Might happen if the local user is not a host or server.
		UE_LOG_GAME_TELEMETRY(Log, "Local user is not a host or server, canceled.")
		return;
	}

	// Get Online Session.
	const TSharedPtr<FNamedOnlineSession> NamedOnlineSession = GetGameOnlineSession();
	if (!NamedOnlineSession)
	{
		UE_LOG_GAME_TELEMETRY(Log, "Online session is invalid. Sending event as offline game.")
	}

	// Get winner team ID.
	// Team ID can be invalid if the current match ended prematurely.
	const AAccelByteWarsGameState* ByteWarsGameState = Cast<AAccelByteWarsGameState>(GameStateBase);
	if (!ensure(ByteWarsGameState))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Current GameState is not AAccelByteWarsGameState, canceled.")
		return;
	}
	const int32 WinnerTeamId = ByteWarsGameState->GetWinnerTeamId();
	const FMatchWinner MatchWinner(FString::FromInt(WinnerTeamId));

	// Send event.
	const bool bResult = GameStandardEventInterface->SendMatchInfoEndedEvent(
		LocalUserNum,
		CurrentMatchInfoId,
		FMatchEndReason(Reason),
		NamedOnlineSession,
		MatchWinner);
	UE_LOG_GAME_TELEMETRY(Log, "Sent succeeded: %s. Match Info ID: %s", *FString(bResult ? TEXT("TRUE") : TEXT("FALSE")), *CurrentMatchInfoId.ToString())

	// Reset stored MatchInfoId.
	CurrentMatchInfoId = FMatchInfoId();
}

void UGameTelemetrySubsystem::OnPlayerEnteredMatch(const FUniqueNetIdPtr PlayerNetId) const
{
	// Only run if OnGameStarted has been executed.
	if (CurrentMatchInfoId.IsEmpty())
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Match info ID is empty. OnGameStarted might not been called, yet. Canceled.")
		return;
	}

	// Get Online Session.
	const TSharedPtr<FNamedOnlineSession> NamedOnlineSession = GetGameOnlineSession();
	if (!NamedOnlineSession)
	{
		UE_LOG_GAME_TELEMETRY(Log, "Online session is invalid. Sending event as offline game.")
	}

	// Get player team.
	const int32 PlayerTeamId = GetPlayerTeamId(PlayerNetId);
	if (PlayerTeamId < 0)
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Player team ID is invalid. Check log prior to this. Canceled.")
		return;
	}
	const FMatchTeam MatchTeam(FString::FromInt(PlayerTeamId));

	// Get AB user ID.
	if (!PlayerNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Player unique net ID is not AccelByte ID. Canceled.")
		return;
	}
	
	const FUniqueNetIdAccelByteUserPtr PlayerAbNetId = FUniqueNetIdAccelByteUser::TryCast(*PlayerNetId);
	if (!PlayerAbNetId.IsValid())
	{
		UE_LOG_GAME_TELEMETRY(Warning, "Player unique net ID is not AccelByte ID. Canceled.")
		return;
	}

	// Send event.
	const bool bResult = GameStandardEventInterface->SendMatchInfoPlayerEvent(
		LocalUserNum,
		PlayerAbNetId,
		CurrentMatchInfoId,
		NamedOnlineSession,
		MatchTeam);
	UE_LOG_GAME_TELEMETRY(Log, "Sent succeeded: %s. Match Info ID: %s", *FString(bResult ? TEXT("TRUE") : TEXT("FALSE")), *CurrentMatchInfoId.ToString())
}

void UGameTelemetrySubsystem::OnEntityDestroyed(
	const FString& DestroyedEntityType,
	const FUniqueNetIdPtr DestroyedPlayerId,
	const FString& DestroyedEntityId,
	const FVector& DestroyedLocation,
	const FString& SourceEntityType,
	const FString& SourceEntityId) const
{
	// Get entity type.
	const FEntityType EntityType = FEntityType(DestroyedEntityType);

	// Get destroyed user ID or entity ID.
	FUniqueNetIdAccelByteUserPtr DestroyedEntityAbNetId = nullptr;
	FEntityId EntityId = {};
	if (DestroyedEntityType.Equals(ENTITY_TYPE_PLAYER))
	{
		if (DestroyedPlayerId && DestroyedPlayerId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
		{
			// Set to AB user ID.
			DestroyedEntityAbNetId = FUniqueNetIdAccelByteUser::TryCast(*DestroyedPlayerId);
			if (!DestroyedEntityAbNetId.IsValid())
			{
				UE_LOG_GAME_TELEMETRY(Warning, "Player unique net ID is not AccelByte ID. Canceled.")
				return;
			}
		}
		else if (GetWorld()->GetNetMode() == ENetMode::NM_Standalone)
		{
			// Set to dummy ID if this is a non-player. Can happen on single player.
			DestroyedEntityAbNetId = DummyAbId;
		}
		else
		{
			UE_LOG_GAME_TELEMETRY(Warning, "Player unique net ID is not AccelByte ID nor this is a single player game. Canceled.")
			return;
		}
	}
	else
	{
		EntityId = FEntityId(DestroyedEntityId);
	}

	// Set optional data.
	FAccelByteModelsEntityDeadOptPayload Optional = {};

	// Set death location.
	Optional.DeathLocation = DestroyedLocation.ToString();

	// Set death type.
	Optional.DeathType = SourceEntityType;

	// Set death source.
	Optional.DeathSource = SourceEntityId;

	// Send event.
	const bool bResult = GameStandardEventInterface->SendEntityDeadEvent(
		LocalUserNum,
		EntityType,
		EntityId,
		DestroyedEntityAbNetId,
		Optional);
	UE_LOG_GAME_TELEMETRY(Log, "Sent succeeded: %s", *FString(bResult ? TEXT("TRUE") : TEXT("FALSE")))
}

TSharedPtr<FNamedOnlineSession> UGameTelemetrySubsystem::GetGameOnlineSession() const
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

FString UGameTelemetrySubsystem::GetFormattedGameMode(TSharedPtr<FNamedOnlineSession> NamedOnlineSession) const
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

int32 UGameTelemetrySubsystem::GetPlayerTeamId(const FUniqueNetIdPtr PlayerNetId) const
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

bool UGameTelemetrySubsystem::CompareAccelByteUniqueId(
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
