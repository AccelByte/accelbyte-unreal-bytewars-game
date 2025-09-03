// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionP2POnlineSession.h"

#include "MatchSessionP2PLog.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-RegisterOnlineDelegates
// @@@MULTISNIP BindFindSessionDelegate {"selectedLines": ["1-2", "19-23"]}
void UMatchSessionP2POnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Game Session delegates
	GetABSessionInt()->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	GetABSessionInt()->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	// Match session delegates
	GetSessionInt()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);

	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-ClearOnlineDelegates
// @@@MULTISNIP UnbindFindSessionDelegate {"selectedLines": ["1-2", "12-13"]}
void UMatchSessionP2POnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	GetSessionInt()->OnFindSessionsCompleteDelegates.RemoveAll(this);
}
// @@@SNIPEND

#pragma region "Game Session Essentials"
// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-TravelToSession
bool UMatchSessionP2POnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get session info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("The session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("The session info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("The session info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Can't find player controller with the session's local owner's unique ID"));
		return false;
	}

	AAccelByteWarsPlayerController* AbPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AbPlayerController)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Player controller is not (derived from) AAccelByteWarsPlayerController"));
		return false;
	}

	FString ServerAddress = "";

	// If local user is not the P2P host -> connect to host
	if (!(AbSessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P && Session->bHosting)) 
	{
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Host is not a P2P host, traveling to host"));
		GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);
		if (ServerAddress.IsEmpty())
		{
			UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Can't find session's server address"));
			return false;
		}
	}
	else
	{
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Host is a P2P host, traveling as listen server"));
		ServerAddress = "MainMenu?listen";
	}

	if (!bIsInSessionServer)
	{
		AbPlayerController->DelayedClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Already in session's server"));
	}

	return true;
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnSessionServerUpdateReceived
void UMatchSessionP2POnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	if (bLeavingSession)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("called but leave session is currently running. Canceling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnSessionServerErrorReceived
void UMatchSessionP2POnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnJoinSessionComplete
void UMatchSessionP2POnlineSession::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	Super::OnJoinSessionComplete(SessionName, Result);

	TravelToSession(SessionName);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnLeaveSessionComplete
void UMatchSessionP2POnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnLeaveSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnCreateSessionComplete
void UMatchSessionP2POnlineSession::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnCreateSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		// attempt to travel -> P2P host will need to travel as listen server right now
		TravelToSession(SessionName);
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-HandleDisconnectInternal
bool UMatchSessionP2POnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
// @@@SNIPEND
#pragma endregion 

#pragma region "Match Session Essentials"
// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-CreateMatchSession
void UMatchSessionP2POnlineSession::CreateMatchSession(
	const int32 LocalUserNum,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType, const EGameStyle GameStyle)
{
	FOnlineSessionSettings SessionSettings;
	// Set a flag so we can request a filtered session from backend
	SessionSettings.Set(GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION);

	// flag to signify the server which game mode to use
	SessionSettings.Set(GAMESETUP_GameModeCode, MatchSessionTargetGameModeMap[{ GameModeType, GameStyle }]);

	// Get match session template name based on game mode type
	FString MatchTemplateName = MatchSessionTemplateNameMap[{EGameModeNetworkType::P2P, GameModeType}];

	// Override match session template name if applicable.
	if (!UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride();
	}

	CreateSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchTemplateName);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-FindSessions
void UMatchSessionP2POnlineSession::FindSessions(
	const int32 LocalUserNum,
	const int32 MaxQueryNum,
	const bool bForce)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	if (SessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Currently searching"))
		return;
	}

	// check cache
	if (!bForce && MaxQueryNum <= SessionSearch->SearchResults.Num())
	{
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Cache found"))

		// return cache
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnFindSessionsComplete(true);
		}));
		return;
	}

	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
	SessionSearch->MaxSearchResults = MaxQueryNum;
	SessionSearch->SearchResults.Empty();
	LocalUserNumSearching = LocalUserNum;

	// reset
	SessionSearch->QuerySettings = FOnlineSearchSettings();

	// Request a filtered session from backend based on the flag we set on CreateSession_Caller
	SessionSearch->QuerySettings.Set(GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION, EOnlineComparisonOp::Equals);

	if (!GetSessionInt()->FindSessions(LocalUserNum, SessionSearch))
	{
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnFindSessionsComplete(false);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionP2POnlineSession.cpp-OnFindSessionsComplete
void UMatchSessionP2POnlineSession::OnFindSessionsComplete(bool bSucceeded)
{
	UE_LOG_MATCHSESSIONP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		// Remove owned session from result if exists
		const FUniqueNetIdPtr LocalUserNetId = GetIdentityInt()->GetUniquePlayerId(LocalUserNumSearching);
		SessionSearch->SearchResults.RemoveAll([this, LocalUserNetId](const FOnlineSessionSearchResult& Element)
		{
			return CompareAccelByteUniqueId(
				FUniqueNetIdRepl(LocalUserNetId),
				FUniqueNetIdRepl(Element.Session.OwningUserId));
		});

		// Trigger immediately if the results are empty at this point.
		if (SessionSearch->SearchResults.IsEmpty())
		{
			OnFindSessionsCompleteDelegates.Broadcast({}, true);
			return;
		}

		// Get owner’s user info for queried user info.
		TArray<FUniqueNetIdRef> UserIds;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UserIds.AddUnique(SearchResult.Session.OwningUserId->AsShared());
		}

		// Trigger to query user info
		if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
		{
			StartupSubsystem->QueryUserInfo(
				LocalUserNumSearching,
				UserIds,
				FOnQueryUsersInfoCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryUserInfoForFindSessionComplete));
		}
	}
	else
	{
		OnFindSessionsCompleteDelegates.Broadcast({}, false);
	}
}
// @@@SNIPEND

void UMatchSessionP2POnlineSession::OnQueryUserInfoForFindSessionComplete(
	const FOnlineError& Error,
	const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
{
	UE_LOG_MATCHSESSIONP2P(Log, TEXT("succeeded: %s"), *FString(Error.bSucceeded ? "TRUE": "FALSE"))

	if (Error.bSucceeded)
	{
		const TArray<FMatchSessionEssentialInfo> MatchSessionSearchResult = SimplifySessionSearchResult(
			SessionSearch->SearchResults,
			UsersInfo,
			MatchSessionTemplateNameMap);

		OnFindSessionsCompleteDelegates.Broadcast(MatchSessionSearchResult, true);
	}
	else
	{
		OnFindSessionsCompleteDelegates.Broadcast({}, false);
	}
}
#pragma endregion 
