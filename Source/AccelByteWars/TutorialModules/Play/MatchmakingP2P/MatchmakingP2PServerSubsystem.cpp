// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PServerSubsystem.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/GameStates/AccelByteWarsGameState.h"

// @@@SNIPSTART MatchmakingP2PServerSubsystem.cpp-Initialize
void UMatchmakingP2PServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UMatchmakingP2POnlineSession>(BaseOnlineSession);

	GetABSessionInt()->OnServerReceivedSessionDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PServerSubsystem.cpp-Deinitialize
void UMatchmakingP2PServerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GetABSessionInt()->OnServerReceivedSessionDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingP2PServerSubsystem.cpp-OnServerSessionReceived
// @@@MULTISNIP SessionSettings {"highlightedLines": "{36-53}"}
void UMatchmakingP2PServerSubsystem::OnServerSessionReceived(FName SessionName)
{
	Super::OnServerSessionReceived(SessionName);
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

#pragma region "Assign game mode based on SessionTemplateName from backend"
	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("World is invalid"));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Game State is invalid"));
		return;
	}

	AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Game State is not derived from AAccelByteWarsGameState"));
		return;
	}

	// Get Game Session
	if (MatchmakingOnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Is not a game session"));
		return;
	}

	const FNamedOnlineSession* Session = MatchmakingOnlineSession->GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("Session is invalid"));
		return;
	}

	FString RequestedGameModeCode = TEXT(""), SessionTemplateName = TEXT("");
	Session->SessionSettings.Get(GAMESETUP_GameModeCode, RequestedGameModeCode);
	Session->SessionSettings.Get(SETTING_SESSION_MATCHPOOL, SessionTemplateName);
	if (!RequestedGameModeCode.IsEmpty())
	{
		AbGameState->AssignGameMode(RequestedGameModeCode);
	}
	else if (!SessionTemplateName.IsEmpty())
	{
		AbGameState->AssignGameMode(MatchmakingOnlineSession->TargetGameModeMap[SessionTemplateName]);
	}
#pragma endregion

	// Query all currently registered user's info
	AuthenticatePlayer_OnRefreshSessionComplete(true);
}
// @@@SNIPEND
