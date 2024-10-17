// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingDSServerSubsystem.h"

#include "MatchmakingDSLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/GameStates/AccelByteWarsGameState.h"

// @@@SNIPSTART MatchmakingDSServerSubsystem.cpp-Deinitialize
void UMatchmakingDSServerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GetABSessionInt()->OnServerReceivedSessionDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSServerSubsystem.cpp-Initialize
void UMatchmakingDSServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UMatchmakingDSOnlineSession>(BaseOnlineSession);

	GetABSessionInt()->OnServerReceivedSessionDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
}
// @@@SNIPEND

// @@@SNIPSTART MatchmakingDSServerSubsystem.cpp-OnServerSessionReceived
// @@@MULTISNIP SessionSettings {"highlightedLines": "{36-53}"}
void UMatchmakingDSServerSubsystem::OnServerSessionReceived(FName SessionName)
{
	Super::OnServerSessionReceived(SessionName);
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

#pragma region "Assign game mode based on SessionTemplateName from backend"
	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("World is invalid"));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Game State is invalid"));
		return;
	}

	AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Game State is not derived from AAccelByteWarsGameState"));
		return;
	}

	// Get Game Session
	if (MatchmakingOnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Is not a game session"));
		return;
	}

	const FNamedOnlineSession* Session = MatchmakingOnlineSession->GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session is invalid"));
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
