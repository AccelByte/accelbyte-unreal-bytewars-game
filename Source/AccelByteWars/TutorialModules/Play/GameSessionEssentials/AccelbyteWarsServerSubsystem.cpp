// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelbyteWarsServerSubsystem.h"

#include "GameSessionEssentialsLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

#pragma region "Game specific"
void UAccelbyteWarsServerSubsystem::OnAuthenticatePlayerComplete_PrePlayerSetup(
	APlayerController* PlayerController)
{
	Super::OnAuthenticatePlayerComplete_PrePlayerSetup(PlayerController);
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

#pragma region "figure out if this is a matchmaking / custom"
	const FNamedOnlineSession* Session = OnlineSession->GetSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	if (!Session)
	{
		UE_LOG_GAMESESSION(Log, TEXT("Session is invalid, do nothing"));
		return;
	}

	FString RequestedSessionType;
	Session->SessionSettings.Get(GAME_SESSION_REQUEST_TYPE, RequestedSessionType);
	if (RequestedSessionType != GAME_SESSION_REQUEST_TYPE_MATCHSESSION)
	{
		UE_LOG_GAMESESSION(Log, TEXT("Session is not a custom session, do nothing"));
		return;
	}
#pragma endregion 

	// get GameMode
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!ensure(GameMode))
	{
		return;
	}
	const AAccelByteWarsGameMode* AbGameMode = Cast<AAccelByteWarsGameMode>(GameMode);
	if (!ensure(AbGameMode))
	{
		return;
	}

	// get PlayerState
	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!ensure(PlayerState))
	{
		return;
	}
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
	if (!ensure(AbPlayerState))
	{
		return;
	}

	AbGameMode->AssignTeamManually(AbPlayerState->TeamId);
}
#pragma endregion 

void UAccelbyteWarsServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

	UOnlineSession* OnlineSessionBase = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(OnlineSessionBase))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSession>(OnlineSessionBase);

	GetABSessionInt()->OnServerReceivedSessionDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
}

void UAccelbyteWarsServerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GetABSessionInt()->OnServerReceivedSessionDelegates.RemoveAll(this);
}

void UAccelbyteWarsServerSubsystem::OnServerSessionReceived(FName SessionName)
{
	Super::OnServerSessionReceived(SessionName);
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

#pragma region "Assign game mode based on session info from backend"
	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("World is invalid"));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Game State is invalid"));
		return;
	}

	AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Game State is not derived from AAccelByteWarsGameState"));
		return;
	}

	// Get Game Session
	if (OnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Is not a game session"));
		return;
	}

	const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Session is invalid"));
		return;
	}

	// Get game related info from session
	FString RequestedGameModeCode;
	FString SessionTemplateName;
	Session->SessionSettings.Get(GAMESETUP_GameModeCode, RequestedGameModeCode);
	Session->SessionSettings.Get(SETTING_SESSION_MATCHPOOL, SessionTemplateName);

	// Try getting manually set game rules
	bool bIsCustomGame = false;
	if (Session->SessionSettings.Get(GAMESETUP_IsCustomGame, bIsCustomGame) && bIsCustomGame)
	{
		AbGameState->AssignCustomGameMode(&Session->SessionSettings);
	}
	// If not complete, try getting requested game mode
	else if (!RequestedGameModeCode.IsEmpty())
	{
		AbGameState->AssignGameMode(RequestedGameModeCode);
	}
	// if not, try to assign game mode base on Match Pool
	else if (!SessionTemplateName.IsEmpty())
	{
		AbGameState->AssignGameMode(OnlineSession->MatchmakingTargetGameModeMap[SessionTemplateName]);
	}
#pragma endregion

	// Query all currently registered user's info
	AuthenticatePlayer_OnRefreshSessionComplete(true);
}
