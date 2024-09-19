// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionDSServerSubsystem.h"

#include "MatchSessionDSLog.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "GameFramework/PlayerState.h"

#pragma region "Game specific"
void UMatchSessionDSServerSubsystem::OnAuthenticatePlayerComplete_PrePlayerSetup(
	APlayerController* PlayerController)
{
	Super::OnAuthenticatePlayerComplete_PrePlayerSetup(PlayerController);

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

void UMatchSessionDSServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UMatchSessionDSOnlineSession>(BaseOnlineSession);

	GetABSessionInt()->OnServerReceivedSessionDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
}

void UMatchSessionDSServerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GetABSessionInt()->OnServerReceivedSessionDelegates.RemoveAll(this);
}

void UMatchSessionDSServerSubsystem::OnServerSessionReceived(FName SessionName)
{
	Super::OnServerSessionReceived(SessionName);
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

#pragma region "Assign game mode based on SessionTemplateName from backend"
	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("World is invalid"));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Game State is invalid"));
		return;
	}

	AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Game State is not derived from AAccelByteWarsGameState"));
		return;
	}

	// Get Game Session
	if (OnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Is not a game session"));
		return;
	}

	const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Session is invalid"));
		return;
	}

	// Get game related info from session
	FString RequestedGameModeCode;
	Session->SessionSettings.Get(GAMESETUP_GameModeCode, RequestedGameModeCode);

	// Try getting manually set game rules, if GAMESETUP_DisplayName empty, go to the next flow
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
#pragma endregion

	// Query all currently registered user's info
	AuthenticatePlayer_OnRefreshSessionComplete(true);
}
