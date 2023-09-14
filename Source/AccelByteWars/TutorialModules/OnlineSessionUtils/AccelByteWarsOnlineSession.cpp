// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsOnlineSession.h"

#include "AccelByteWarsOnlineSessionLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"

void UAccelByteWarsOnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// Session Essentials
	GetSessionInt()->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	GetSessionInt()->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	GetSessionInt()->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnLeaveSessionComplete));

	// Game Session Essentials
	GetABSessionInt()->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	GetABSessionInt()->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);

	// Matchmaking Essentials
	GetSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);

	// Match Session Essentials
	GetSessionInt()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);
	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
}

void UAccelByteWarsOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	// Session Essentials
	GetSessionInt()->ClearOnCreateSessionCompleteDelegates(this);
	GetSessionInt()->ClearOnJoinSessionCompleteDelegates(this);
	GetSessionInt()->ClearOnDestroySessionCompleteDelegates(this);

	// Game Session Essentials
	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);

	// Matchmaking Essentials
	GetSessionInt()->OnMatchmakingCompleteDelegates.RemoveAll(this);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.RemoveAll(this);

	// Match Session Essentials
	GetSessionInt()->OnFindSessionsCompleteDelegates.RemoveAll(this);
}

#pragma region "Session Essentials"
FNamedOnlineSession* UAccelByteWarsOnlineSession::GetSession(const FName SessionName)
{
	return GetSessionInt()->GetNamedSession(SessionName);
}

EAccelByteV2SessionType UAccelByteWarsOnlineSession::GetSessionType(const FName SessionName)
{
	const FNamedOnlineSession* OnlineSession = GetSession(SessionName);
	if (!OnlineSession)
	{
		return EAccelByteV2SessionType::Unknown;
	}

	const FOnlineSessionSettings& OnlineSessionSettings = OnlineSession->SessionSettings;

	return GetABSessionInt()->GetSessionTypeFromSettings(OnlineSessionSettings);
}

FName UAccelByteWarsOnlineSession::GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType)
{
	FName SessionName = FName();

	switch (SessionType)
	{
	case EAccelByteV2SessionType::GameSession:
		SessionName = GameSessionName;
		break;
	case EAccelByteV2SessionType::PartySession:
		SessionName = PartySessionName;
		break;
	default: ;
	}

	return SessionName;
}

void UAccelByteWarsOnlineSession::CreateSession(
	const int32 LocalUserNum,
	FName SessionName,
	FOnlineSessionSettings SessionSettings,
	const EAccelByteV2SessionType SessionType,
	const FString& SessionTemplateName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface is null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreateSessionComplete(SessionName, false);
		}));
		return;
	}
	if (SessionTemplateName.IsEmpty())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Template Name can't be empty"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreateSessionComplete(SessionName, false);
		}));
		return;
	}

#pragma region "Setup Session Settings"
	// Session Template Name
	SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, SessionTemplateName);

	// Session type
	if (SessionType != EAccelByteV2SessionType::Unknown)
	{
		SessionSettings.Set(SETTING_SESSION_TYPE, GetPredefinedSessionNameFromType(SessionType).ToString());
	}

	// Set local server name for matchmaking request if any.
	// This is useful if you want to try matchmaking using local dedicated server.
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		FString ServerName;
		FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
		if (!ServerName.IsEmpty())
		{
			UE_LOG_ONLINESESSION(Log, TEXT("Requesting to use server with name: %s"), *ServerName)
			SessionSettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName);
		}
	}
#pragma endregion

	// if session exist locally -> destroy session first
	if (GetSession(SessionName))
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Session exist locally, leaving session first"))

		// remove from delegate if exist
		if (OnLeaveSessionForCreateSessionCompleteDelegateHandle.IsValid())
		{
			OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForCreateSessionCompleteDelegateHandle);
			OnLeaveSessionForCreateSessionCompleteDelegateHandle.Reset();
		}

		OnLeaveSessionForCreateSessionCompleteDelegateHandle = OnLeaveSessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeaveSessionForCreateSessionComplete, LocalUserNum, SessionSettings);
		LeaveSession(SessionName);
	}
	else
	{
		if (!GetSessionInt()->CreateSession(LocalUserNum, SessionName, SessionSettings))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnCreateSessionComplete(SessionName, false);
			}));
		}
	}
}

void UAccelByteWarsOnlineSession::JoinSession(
	const int32 LocalUserNum,
	FName SessionName,
	const FOnlineSessionSearchResult& SearchResult)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		}));
		return;
	}

	// If session exist -> destroy first then join
	if (GetSession(SessionName))
	{
		// remove from delegate if exist
		if (OnLeaveSessionForJoinSessionCompleteDelegateHandle.IsValid())
		{
			OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForJoinSessionCompleteDelegateHandle);
			OnLeaveSessionForJoinSessionCompleteDelegateHandle.Reset();
		}

		OnLeaveSessionForJoinSessionCompleteDelegateHandle = OnLeaveSessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeaveSessionForJoinSessionComplete, LocalUserNum, SearchResult);
		LeaveSession(SessionName);
	}
	else
	{
		if (!GetSessionInt()->JoinSession(LocalUserNum, SessionName, SearchResult))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}));
		}
	}
}

void UAccelByteWarsOnlineSession::LeaveSession(FName SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeaveSessionComplete(SessionName, false);
		}));
		return;
	}

	if (GetSession(SessionName))
	{
		if (!GetABSessionInt()->DestroySession(SessionName))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnLeaveSessionComplete(SessionName, false);
			}));
		}
		else
		{
			bLeaveSessionRunning = true;
		}
	}
	else
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Not in session"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeaveSessionComplete(SessionName, true);
		}));
	}
}

void UAccelByteWarsOnlineSession::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

#pragma region "P2P Match Session implementation"
	/*
	 * Attempt to travel to session. Used for Match Session P2P.
	 * If the current session is not P2P, the TravelToSession will simply returns false.
	 */
	if (SessionName == GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession))
	{
		TravelToSession(SessionName);
	}
#pragma endregion 

	OnCreateSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(Result == EOnJoinSessionCompleteResult::Success ? "TRUE": "FALSE"))

#pragma region "P2P Matchmaking implementation"
	/*
	 * Attempt to travel to session. Used for Match Session P2P.
	 * If the current session is not P2P, the TravelToSession will simply returns false.
	 */
	if (SessionName == GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession))
	{
		TravelToSession(SessionName);
	}
#pragma endregion 

	OnJoinSessionCompleteDelegates.Broadcast(SessionName, Result);
}

void UAccelByteWarsOnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

#pragma region "Game Session implementation"
	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
#pragma endregion 

	bLeaveSessionRunning = false;
	OnLeaveSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnLeaveSessionForCreateSessionComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const FOnlineSessionSettings SessionSettings)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForCreateSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->CreateSession(LocalUserNum, SessionName, SessionSettings))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnCreateSessionComplete(SessionName, false);
			}));
		}
	}
	else
	{
		// Leave Session failed, execute complete delegate as failed
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreateSessionComplete(SessionName, false);
		}));
	}
}

void UAccelByteWarsOnlineSession::OnLeaveSessionForJoinSessionComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const FOnlineSessionSearchResult SearchResult)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForJoinSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->JoinSession(LocalUserNum, SessionName, SearchResult))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("failed to execute"))
			ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}));
		}
	}
	else
	{
		// Leave Session failed, execute complete delegate as failed
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		}));
	}
}
#pragma endregion 

#pragma region "Game Session Essentials"
void UAccelByteWarsOnlineSession::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("User interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
		{
			UE_LOG_ONLINESESSION(Log, TEXT("trigger OnComplete"))
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	// Some data does not exist in cache, query everything
	else
	{
		// Bind delegate
		if (OnQueryUserInfoCompleteDelegateHandle.IsValid())
		{
			GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
			OnQueryUserInfoCompleteDelegateHandle.Reset();
		}
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

void UAccelByteWarsOnlineSession::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	const TArray<const FBaseUserInfo*> UserInfo;
	if (DSRetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete, UserInfo]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	else
	{
		// gather user ids
		TArray<FString> AbUserIds;
		for (const FUniqueNetIdRef& UserId : UserIds)
		{
			const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*UserId);
			const FString AbUserId = AbUniqueNetId->GetAccelByteId();
			if (!AbUserId.IsEmpty())
			{
				AbUserIds.Add(AbUserId);
			}
		}

		AccelByte::FRegistry::User.BulkGetUserInfo(
			AbUserIds,
			THandler<FListBulkUserInfo>::CreateWeakLambda(this, [OnComplete, this](FListBulkUserInfo UserInfo)
			{
				OnDSQueryUserInfoComplete(UserInfo, OnComplete);
			}),
			FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32, const FString&)
			{
				ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete(FListBulkUserInfo(), OnComplete);
				}));
			})
		);
	}
}

bool UAccelByteWarsOnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get Session Info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Can't find player controller with the session's local owner's Unique Id"));
		return false;
	}

	// Find address
#pragma region "Adapted for P2P and DS"
	FString ServerAddress = "";
	const bool bIsMatchmakingP2PHost = GetABSessionInt()->IsPlayerP2PHost(
		GetLocalPlayerUniqueNetId(PlayerController).ToSharedRef().Get(), SessionName);
	const bool bIsMatchSessionP2PHost =
		AbSessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P && Session->bHosting;
	if (bIsMatchmakingP2PHost || bIsMatchSessionP2PHost)
	{
		// P2P host -> travel as listen server
		UE_LOG_ONLINESESSION(Log, TEXT("Local user is the P2P host, travel as listen server"));
		ServerAddress = "MainMenu?listen";
	}
	else
	{
		// client
		GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);
	}
#pragma endregion 

	if (ServerAddress.IsEmpty())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Can't find session's server address"));
		return false;
	}

	if (!bIsInSessionServer)
	{
		PlayerController->ClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Already in session's server"));
	}

	return true;
}

void UAccelByteWarsOnlineSession::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	// reset delegate handle
	GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
	OnQueryUserInfoCompleteDelegateHandle.Reset();

	if (bSucceeded)
	{
		TArray<FUserOnlineAccountAccelByte*> OnlineUsers;
		if (!RetrieveUserInfoCache(UserIds, OnlineUsers))
		{
			CacheUserInfo(LocalUserNum, UserIds);

			/**
			 * Only include valid users info, in the case of invalid user ids, doesn't exist in backend,
			 * their data simply would not exist in the OnComplete delegate.
			 * Asuumes data does not exist in backend if users info is not in the OnComplete's parameter.
			 */
			for (const FUniqueNetIdRef& UserId : UserIds)
			{
				TSharedPtr<FOnlineUser> OnlineUserPtr = GetUserInt()->GetUserInfo(LocalUserNum, UserId.Get());
				if (OnlineUserPtr.IsValid())
				{
					TSharedPtr<FUserOnlineAccountAccelByte> AbUserPtr = StaticCastSharedPtr<
						FUserOnlineAccountAccelByte>(OnlineUserPtr);
					OnlineUsers.AddUnique(AbUserPtr.Get());
				}
			}
		}
		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

void UAccelByteWarsOnlineSession::OnDSQueryUserInfoComplete(
	const FListBulkUserInfo& UserInfoList,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// reset delegate handle
	OnDSQueryUserInfoCompleteDelegateHandle.Reset();

	if (UserInfoList.Data.IsEmpty())
	{
		OnComplete.ExecuteIfBound(false, {});
	}

	CacheUserInfo(UserInfoList);

	TArray<const FBaseUserInfo*> OnlineUsers;
	for (const FBaseUserInfo& User : UserInfoList.Data)
	{
		OnlineUsers.Add(&User);
	}
	OnComplete.ExecuteIfBound(true, OnlineUsers);
}

void UAccelByteWarsOnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	if (bLeaveSessionRunning)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}

void UAccelByteWarsOnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}

bool UAccelByteWarsOnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
void UAccelByteWarsOnlineSession::StartMatchmaking(
	const APlayerController* PC,
	const FName& SessionName,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// If the player is already in a session, then leave session first.
	if (GetSession(SessionName))
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Already in session. Leaving session first."))
		if (OnLeaveSessionForReMatchmakingCompleteDelegateHandle.IsValid())
		{
			OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForReMatchmakingCompleteDelegateHandle);
			OnLeaveSessionForReMatchmakingCompleteDelegateHandle.Reset();
		}

		OnLeaveSessionForReMatchmakingCompleteDelegateHandle = OnLeaveSessionCompleteDelegates.AddUObject(
			this,
			&ThisClass::OnLeaveSessionForReMatchmakingComplete,
			GetLocalUserNumFromPlayerController(PC),
			NetworkType,
			GameModeType);
		LeaveSession(SessionName);
		return;
	}

	const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
	if (!ensure(PlayerNetId.IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Player UniqueNetId is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// Get match pool id based on game mode type
	const FString MatchPoolId = MatchmakingPoolIdMap[{NetworkType, GameModeType}];
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(
		SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);

	// Set local server name for matchmaking request if any.
	// This is useful if you want to try matchmaking using local dedicated server.
	FString ServerName;
	FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
	if (!ServerName.IsEmpty())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Requesting local server with name: %s"), *ServerName)
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName, EOnlineComparisonOp::Equals);
	}

	if (!GetSessionInt()->StartMatchmaking(
		USER_ID_TO_MATCHMAKING_USER_ARRAY(PlayerNetId.ToSharedRef()),
		SessionName,
		FOnlineSessionSettings(),
		MatchmakingSearchHandle,
		FOnStartMatchmakingComplete::CreateUObject(this, &ThisClass::OnStartMatchmakingComplete)))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}

void UAccelByteWarsOnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->CurrentMatchmakingSearchHandle.IsValid() &&
		GetABSessionInt()->CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Searching player ID is not valid."));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!GetSessionInt()->CancelMatchmaking(
		*GetABSessionInt()->CurrentMatchmakingSearchHandle->SearchingPlayerId,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
	}
}

void UAccelByteWarsOnlineSession::OnStartMatchmakingComplete(
	FName SessionName,
	const FOnlineError& ErrorDetails,
	const FSessionMatchmakingResults& Results)
{
	UE_LOG_ONLINESESSION(
		Log,
		TEXT("succeeded: %s | error: (%s) %s"),
		*FString(ErrorDetails.bSucceeded ? "TRUE": "FALSE"),
		*ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString())

	OnStartMatchmakingCompleteDelegates.Broadcast(SessionName, ErrorDetails.bSucceeded);
}

void UAccelByteWarsOnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->CurrentMatchmakingSearchHandle;
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when the MM finished just as we are about to cancel it*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->SearchingPlayerId.IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("There is no match result returned."));
		OnMatchmakingCompleteDelegates.Broadcast(SessionName, false);
		return;
	}

	OnMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);

	// Get searching player
	const int32 LocalUserNum =
		GetLocalUserNumFromPlayerController(GetPlayerControllerByUniqueNetId(CurrentMatchmakingSearchHandle->SearchingPlayerId));

	// Join the first session from matchmaking result.
	JoinSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		CurrentMatchmakingSearchHandle->SearchResults[0]);
}

void UAccelByteWarsOnlineSession::OnBackfillProposalReceived(
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	// Safety
	if (!ensure(GetABSessionInt().IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Interface is not valid."));
		return;
	}

	// Accept backfill proposal.
	GetABSessionInt()->AcceptBackfillProposal(
		NAME_GameSession,
		Proposal,
		false,
		FOnAcceptBackfillProposalComplete::CreateWeakLambda(this, [this](bool bSucceeded)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s To accept backfill."), *FString(bSucceeded ? "TRUE": "FALSE"));
		OnAcceptBackfillProposalCompleteDelegates.Broadcast(bSucceeded);
	}));
}

void UAccelByteWarsOnlineSession::OnLeaveSessionForReMatchmakingComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForReMatchmakingCompleteDelegateHandle);

	if (bSucceeded)
	{
		// Retry matchmaking.
		const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
		if (!ensure(PC))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("PlayerController is null."));
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
			return;
		}

		StartMatchmaking(PC, SessionName, NetworkType, GameModeType);
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave session, starting matchmake anyway"));
		OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
	}
}
#pragma endregion 

#pragma region "Match Session Essentials"
void UAccelByteWarsOnlineSession::CreateMatchSession(
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
		MatchSessionTargetGameModeMap[{NetworkType, GameModeType}]);

	CreateSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchSessionTemplateNameMap[{NetworkType, GameModeType}]);
}

void UAccelByteWarsOnlineSession::FindSessions(
	const int32 LocalUserNum,
	const int32 MaxQueryNum,
	const bool bForce)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("called"))

	if (SessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Currently searching"))
		return;
	}

	// check cache
	if (!bForce && MaxQueryNum <= SessionSearch->SearchResults.Num())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Cache found"))

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

void UAccelByteWarsOnlineSession::OnFindSessionsComplete(bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

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

void UAccelByteWarsOnlineSession::OnQueryUserInfoForFindSessionComplete(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		const TArray<FMatchSessionEssentialInfo> MatchSessionSearchResult = SimplifySessionSearchResult(
			SessionSearch->SearchResults,
			UsersInfo,
			MatchSessionTemplateNameMap
		);

		OnFindSessionsCompleteDelegates.Broadcast(MatchSessionSearchResult, true);
	}
	else
	{
		OnFindSessionsCompleteDelegates.Broadcast({}, false);
	}
}
#pragma endregion
