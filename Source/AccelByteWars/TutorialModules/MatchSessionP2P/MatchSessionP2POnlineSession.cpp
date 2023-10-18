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

void UMatchSessionP2POnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// Game Session delegates
	GetABSessionInt()->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	GetABSessionInt()->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Match Session delegates
	GetSessionInt()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);

	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
}

void UMatchSessionP2POnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	GetSessionInt()->OnFindSessionsCompleteDelegates.RemoveAll(this);
}

void UMatchSessionP2POnlineSession::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	Super::OnJoinSessionComplete(SessionName, Result);

	TravelToSession(SessionName);
}

void UMatchSessionP2POnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnLeaveSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
}

void UMatchSessionP2POnlineSession::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	Super::OnCreateSessionComplete(SessionName, bSucceeded);

	if (bSucceeded)
	{
		// attempt to travel -> P2P host will need to travel as listen server right now
		TravelToSession(SessionName);
	}
}

#pragma region "Game Session Essentials"
void UMatchSessionP2POnlineSession::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("User interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	// Some data does not exist in cache, query everything
	else
	{
		// Bind delegate
		OnQueryUserInfoCompleteDelegateHandle = GetUserInt()->OnQueryUserInfoCompleteDelegates->AddWeakLambda(
			this, [OnComplete, this](
				int32 LocalUserNum,
				bool bSucceeded,
				const TArray<FUniqueNetIdRef>& UserIds,
				const FString& ErrorMessage)
			{
				OnQueryUserInfoComplete(LocalUserNum, bSucceeded, UserIds, ErrorMessage, OnComplete);
			});

		if (!GetUserInt()->QueryUserInfo(LocalUserNum, UserIds))
		{
			OnQueryUserInfoComplete(LocalUserNum, false, UserIds, "", OnComplete);
		}
	}
}

bool UMatchSessionP2POnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get Session Info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Session Info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Session Info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Can't find player controller with the session's local owner's Unique Id"));
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
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Is not P2P host, travelling to host"));
		GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);
		if (ServerAddress.IsEmpty())
		{
			UE_LOG_MATCHSESSIONP2P(Warning, TEXT("Can't find session's server address"));
			return false;
		}
	}
	else
	{
		UE_LOG_MATCHSESSIONP2P(Log, TEXT("Is P2P host, travelling as listen server"));
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

void UMatchSessionP2POnlineSession::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	// reset delegate handle
	GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
	OnQueryUserInfoCompleteDelegateHandle.Reset();

	if (bSucceeded)
	{
		// Cache the result.
		CacheUserInfo(LocalUserNum, UserIds);

		// Retrieve the result from cache.
		TArray<FUserOnlineAccountAccelByte*> OnlineUsers;
		RetrieveUserInfoCache(UserIds, OnlineUsers);

		// Only include valid users info only.
		OnlineUsers.RemoveAll([](const FUserOnlineAccountAccelByte* Temp)
		{
			return !Temp || !Temp->GetUserId()->IsValid();
		});

		UE_LOG_MATCHSESSIONP2P(Log,
			TEXT("Queried users info: %d, found valid users info: %d"),
			UserIds.Num(), OnlineUsers.Num());

		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

void UMatchSessionP2POnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	if (bLeaveSessionRunning)
	{
		UE_LOG_MATCHSESSIONP2P(Warning, TEXT("called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}

void UMatchSessionP2POnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}

bool UMatchSessionP2POnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHSESSIONP2P(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
#pragma endregion 

#pragma region "Match Session Essentials"
void UMatchSessionP2POnlineSession::CreateMatchSession(
	const int32 LocalUserNum,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	FOnlineSessionSettings SessionSettings;
	// Set a flag so we can request a filtered session from backend
	SessionSettings.Set(GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION);

	// flag to signify the server which game mode to use
	SessionSettings.Set(
		GAMESETUP_GameModeCode,
		FString(GameModeType == EGameModeType::FFA ? "ELIMINATION-P2P" : "TEAMDEATHMATCH-P2P"));

	CreateSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchSessionTemplateNameMap[{EGameModeNetworkType::P2P, GameModeType}]);
}

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
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
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
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnFindSessionsComplete(false);
		}));
	}
}

void UMatchSessionP2POnlineSession::OnFindSessionsComplete(bool bSucceeded)
{
	UE_LOG_MATCHSESSIONP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		// remove owned session from result if exist
		const FUniqueNetIdPtr LocalUserNetId = GetIdentityInt()->GetUniquePlayerId(LocalUserNumSearching);
		SessionSearch->SearchResults.RemoveAll([this, LocalUserNetId](const FOnlineSessionSearchResult& Element)
		{
			return CompareAccelByteUniqueId(
				FUniqueNetIdRepl(LocalUserNetId),
				FUniqueNetIdRepl(Element.Session.OwningUserId));
		});

		// get owners user info for query user info
		TArray<FUniqueNetIdRef> UserIds;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UserIds.AddUnique(SearchResult.Session.OwningUserId->AsShared());
		}

		// trigger Query User info
		QueryUserInfo(
			LocalUserNumSearching,
			UserIds,
			FOnQueryUsersInfoComplete::CreateUObject(this, &ThisClass::OnQueryUserInfoForFindSessionComplete));
	}
	else
	{
		OnFindSessionsCompleteDelegates.Broadcast({}, false);
	}
}

void UMatchSessionP2POnlineSession::OnQueryUserInfoForFindSessionComplete(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
{
	UE_LOG_MATCHSESSIONP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
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
