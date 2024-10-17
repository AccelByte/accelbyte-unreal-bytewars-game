// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsOnlineSession.h"

#include "AccelByteWarsOnlineSessionLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "Core/AccelByteWebSocketErrorTypes.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

class UStartupSubsystem;

void UAccelByteWarsOnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	IOnlineSessionPtr SessionInt = GetSessionInt();
	if (!ensureMsgf(SessionInt, TEXT("OnlineSession interface is nullptr."))) return;

	FOnlineSessionV2AccelBytePtr ABSessionInt = GetABSessionInt();
	if (!ensureMsgf(ABSessionInt, TEXT("AB OnlineSession interface is nullptr."))) return;

	// Session Essentials
	SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	SessionInt->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	ABSessionInt->AddOnSendSessionInviteCompleteDelegate_Handle(
		FOnSendSessionInviteCompleteDelegate::CreateUObject(this, &ThisClass::OnSendSessionInviteComplete));
	SessionInt->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnLeaveSessionComplete));
	SessionInt->AddOnUpdateSessionCompleteDelegate_Handle(
		FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionComplete));

	ABSessionInt->AddOnV2SessionInviteReceivedDelegate_Handle(
		FOnV2SessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived));
	ABSessionInt->AddOnSessionParticipantsChangeDelegate_Handle(
		FOnSessionParticipantsChangeDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsChange));

	// Game Session Essentials
	ABSessionInt->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	ABSessionInt->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Matchmaking Essentials
	SessionInt->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	SessionInt->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	ABSessionInt->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);

	// Match Session Essentials
	SessionInt->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);
	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;

	// Party Essentials
	ABSessionInt->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreatePartyComplete);
	ABSessionInt->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinPartyComplete);
	ABSessionInt->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeavePartyComplete);
	ABSessionInt->OnSendSessionInviteCompleteDelegates.AddUObject(this, &ThisClass::OnSendPartyInviteComplete);
	ABSessionInt->OnSessionInviteRejectedDelegates.AddUObject(this, &ThisClass::OnPartyInviteRejected);
	ABSessionInt->OnV2SessionInviteReceivedDelegates.AddUObject(this, &ThisClass::OnPartyInviteReceived);
	ABSessionInt->OnKickedFromSessionDelegates.AddUObject(this, &ThisClass::OnKickedFromParty);
	ABSessionInt->OnSessionParticipantsChangeDelegates.AddUObject(this, &ThisClass::OnPartyMembersChange);
	ABSessionInt->OnSessionUpdateReceivedDelegates.AddUObject(this, &ThisClass::OnPartySessionUpdateReceived);

	// Lobby Connection
	GetABIdentityInt()->OnConnectLobbyCompleteDelegates->AddUObject(this, &ThisClass::OnConnectLobbyComplete);

	InitializePartyGeneratedWidgets();
}

void UAccelByteWarsOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	IOnlineSessionPtr SessionInt = GetSessionInt();
	if (!ensureMsgf(SessionInt, TEXT("OnlineSession interface is nullptr."))) return;

	FOnlineSessionV2AccelBytePtr ABSessionInt = GetABSessionInt();
	if (!ensureMsgf(ABSessionInt, TEXT("AB OnlineSession interface is nullptr."))) return;

	// Session Essentials
	SessionInt->ClearOnCreateSessionCompleteDelegates(this);
	SessionInt->ClearOnJoinSessionCompleteDelegates(this);
	ABSessionInt->ClearOnSendSessionInviteCompleteDelegates(this);
	SessionInt->ClearOnDestroySessionCompleteDelegates(this);
	SessionInt->ClearOnUpdateSessionCompleteDelegates(this);

	ABSessionInt->ClearOnV2SessionInviteReceivedDelegates(this);
	ABSessionInt->ClearOnSessionParticipantsChangeDelegates(this);

	// Game Session Essentials
	ABSessionInt->OnSessionServerUpdateDelegates.RemoveAll(this);
	ABSessionInt->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	// Matchmaking Essentials
	SessionInt->OnMatchmakingCompleteDelegates.RemoveAll(this);
	SessionInt->OnCancelMatchmakingCompleteDelegates.RemoveAll(this);
	ABSessionInt->OnBackfillProposalReceivedDelegates.RemoveAll(this);

	// Match Session Essentials
	SessionInt->OnFindSessionsCompleteDelegates.RemoveAll(this);

	// Party Essentials
	ABSessionInt->OnCreateSessionCompleteDelegates.RemoveAll(this);
	ABSessionInt->OnJoinSessionCompleteDelegates.RemoveAll(this);
	ABSessionInt->OnDestroySessionCompleteDelegates.RemoveAll(this);
	ABSessionInt->OnSendSessionInviteCompleteDelegates.RemoveAll(this);
	ABSessionInt->OnSessionInviteRejectedDelegates.RemoveAll(this);
	ABSessionInt->OnV2SessionInviteReceivedDelegates.RemoveAll(this);
	ABSessionInt->OnKickedFromSessionDelegates.RemoveAll(this);
	ABSessionInt->OnSessionParticipantsChangeDelegates.RemoveAll(this);
	ABSessionInt->OnSessionUpdateReceivedDelegates.RemoveAll(this);

	// Lobby Connection
	GetABIdentityInt()->OnConnectLobbyCompleteDelegates->RemoveAll(this);

	DeinitializePartyGeneratedWidgets();
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

// @@@SNIPSTART AccelByteWarsOnlineSession.cpp-CreateSession
// @@@MULTISNIP SetClientVersionAttribute {"selectedLines": ["1-7", "40-47", "58", "87"]}
void UAccelByteWarsOnlineSession::CreateSession(
	const int32 LocalUserNum,
	FName SessionName,
	FOnlineSessionSettings SessionSettings,
	const EAccelByteV2SessionType SessionType,
	const FString& SessionTemplateName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface is null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreateSessionComplete(SessionName, false);
		}));
		return;
	}
	if (SessionTemplateName.IsEmpty())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Template Name can't be empty"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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

	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		// Check for DS version override.
		const FString OverriddenDSVersion = UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride();
		if (!OverriddenDSVersion.IsEmpty()) 
		{
			SessionSettings.Set(SETTING_GAMESESSION_CLIENTVERSION, OverriddenDSVersion);
		}

		// Set local server name for matchmaking request if any.
		// This is useful if you want to try matchmaking using local dedicated server.
		FString ServerName = TEXT("");
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
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnCreateSessionComplete(SessionName, false);
			}));
		}
	}
}
// @@@SNIPEND

void UAccelByteWarsOnlineSession::JoinSession(
	const int32 LocalUserNum,
	FName SessionName,
	const FOnlineSessionSearchResult& SearchResult)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}));
		}
	}
}

void UAccelByteWarsOnlineSession::SendSessionInvite(
	const int32 LocalUserNum,
	FName SessionName,
	const FUniqueNetIdPtr Invitee)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"));

	if (!Invitee.IsValid())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Invitee net id is invalid. Cancelling operation"));
		return;
	}

	GetABSessionInt()->SendSessionInviteToFriend(LocalUserNum, SessionName, *Invitee.Get());
}

void UAccelByteWarsOnlineSession::RejectSessionInvite(
	const int32 LocalUserNum,
	const FOnlineSessionInviteAccelByte& Invite)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"));

	const FUniqueNetIdPtr LocalUserNetId = GetLocalPlayerUniqueNetId(GetPlayerControllerByLocalUserNum(LocalUserNum));
	if (!LocalUserNetId.IsValid())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Local User net id is invalid. Cancelling operation"));
		return;
	}

	GetABSessionInt()->RejectInvite(
		*LocalUserNetId.Get(),
		Invite,
		FOnRejectSessionInviteComplete::CreateUObject(this, &ThisClass::OnRejectSessionInviteComplete));
}

void UAccelByteWarsOnlineSession::LeaveSession(FName SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	// safety
	if (!GetSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeaveSessionComplete(SessionName, true);
		}));
	}
}

void UAccelByteWarsOnlineSession::UpdateSessionJoinability(const FName SessionName, const EAccelByteV2SessionJoinability Joinability)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"));

	FOnlineSessionV2AccelBytePtr ABSessionInt = GetABSessionInt();
	if (!ABSessionInt)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session interface is null"));
		return;
	}

	FNamedOnlineSession* Session = ABSessionInt->GetNamedSession(SessionName);
	if (!Session) 
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session is invalid"));
		return;
	}
	
	Session->SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, UEnum::GetValueAsString(Joinability));
	ABSessionInt->UpdateSession(SessionName, Session->SessionSettings);
}

void UAccelByteWarsOnlineSession::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

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
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(Result == EOnJoinSessionCompleteResult::Success ? "TRUE": "FALSE"))

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

void UAccelByteWarsOnlineSession::OnSendSessionInviteComplete(
	const FUniqueNetId& LocalSenderId,
	FName SessionName,
	bool bSucceeded,
	const FUniqueNetId& InviteeId)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	OnSendSessionInviteCompleteDelegates.Broadcast(LocalSenderId, SessionName, bSucceeded, InviteeId);
}

void UAccelByteWarsOnlineSession::OnRejectSessionInviteComplete(bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	OnRejectSessionInviteCompleteDelegates.Broadcast(bSucceeded);
}

void UAccelByteWarsOnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

#pragma region "Game Session implementation"
	if (bSucceeded)
	{
		bIsInSessionServer = false;
	}
#pragma endregion 

	bLeaveSessionRunning = false;
	OnLeaveSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnUpdateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnUpdateSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	UE_LOG_ONLINESESSION(Log, TEXT("From: %s"), *FromId.ToDebugString())

	OnSessionInviteReceivedDelegates.Broadcast(UserId, FromId, Invite);
}

void UAccelByteWarsOnlineSession::OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member,
	bool bJoined)
{
	UE_LOG_ONLINESESSION(
		Log,
		TEXT("Session name: %s | Member: %s [%s]"),
		*SessionName.ToString(),
		*Member.ToDebugString(),
		*FString(bJoined ? "Joined" : "Left"))

	OnSessionParticipantsChangeDelegates.Broadcast(SessionName, Member, bJoined);
}

void UAccelByteWarsOnlineSession::OnLeaveSessionForCreateSessionComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const FOnlineSessionSettings SessionSettings)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForCreateSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->CreateSession(LocalUserNum, SessionName, SessionSettings))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnCreateSessionComplete(SessionName, false);
			}));
		}
	}
	else
	{
		// Leave Session failed, execute complete delegate as failed
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForJoinSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->JoinSession(LocalUserNum, SessionName, SearchResult))
		{
			UE_LOG_ONLINESESSION(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}));
		}
	}
	else
	{
		// Leave Session failed, execute complete delegate as failed
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		}));
	}
}
#pragma endregion 

#pragma region "Game Session Essentials"
void UAccelByteWarsOnlineSession::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	const TArray<const FBaseUserInfo*> UserInfo;
	if (DSRetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Cache found"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete, UserInfo]()
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
				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete(FListBulkUserInfo(), OnComplete);
				}));
			})
		);
	}
}

bool UAccelByteWarsOnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

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

	AAccelByteWarsPlayerController* AbPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AbPlayerController)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Player controller is not (derived from) AAccelByteWarsPlayerController"));
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
		AbPlayerController->DelayedClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Already in session's server"));
	}

	return true;
}

void UAccelByteWarsOnlineSession::OnDSQueryUserInfoComplete(
	const FListBulkUserInfo& UserInfoList,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

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
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	if (bLeaveSessionRunning)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Called but leave session is currently running. Cancelling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}

void UAccelByteWarsOnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}

bool UAccelByteWarsOnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
#pragma endregion 

#pragma region "Matchmaking Session Essentials"
// @@@SNIPSTART AccelByteWarsOnlineSession.cpp-StartMatchmaking
// @@@MULTISNIP SetClientVersionAttribute {"selectedLines": ["1-6", "71-81", "130"]}
void UAccelByteWarsOnlineSession::StartMatchmaking(
	const APlayerController* PC,
	const FName& SessionName,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
		return;
	}

	// Get match pool id based on game mode type
	FString MatchPoolId = MatchmakingPoolIdMap[{NetworkType, GameModeType}];
	const FString GameModeCode = MatchmakingTargetGameModeMap[MatchPoolId];

	// if not using AMS, remove suffix -ams (internal purpose)
	if(!UTutorialModuleOnlineUtility::GetIsServerUseAMS())
	{
		MatchPoolId = MatchPoolId.Replace(TEXT("-ams"), TEXT(""));
	}
	
	// Override match pool id if applicable.
	if (NetworkType == EGameModeNetworkType::DS && !UTutorialModuleOnlineUtility::GetMatchPoolDSOverride().IsEmpty())
	{
		MatchPoolId = UTutorialModuleOnlineUtility::GetMatchPoolDSOverride();
	}
	else if (NetworkType == EGameModeNetworkType::P2P && !UTutorialModuleOnlineUtility::GetMatchPoolP2POverride().IsEmpty()) 
	{
		MatchPoolId = UTutorialModuleOnlineUtility::GetMatchPoolP2POverride();
	}
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);
	MatchmakingSearchHandle->QuerySettings.Set(GAMESETUP_GameModeCode, GameModeCode, EOnlineComparisonOp::Equals);

	// Check for DS version override.
	const FString OverriddenDSVersion = UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride();
	if (!OverriddenDSVersion.IsEmpty())
	{
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_CLIENTVERSION, OverriddenDSVersion, EOnlineComparisonOp::Equals);
	}

	// Set local server name for matchmaking request if any.
	// This is useful if you want to try matchmaking using local dedicated server.
	FString ServerName = TEXT("");
	FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
	if (!ServerName.IsEmpty())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Requesting local server with name: %s"), *ServerName)
		MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName, EOnlineComparisonOp::Equals);
	}

	// include region preferences into matchmaking setting
	if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
		this,
		true))
	{
		if (!ModuleDataAsset->IsStarterModeActive())
		{
			UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
			ensure(GameInstance);

			URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
			if(RegionPreferencesSubsystem != nullptr)
			{
				TArray<FString> EnabledRegion = RegionPreferencesSubsystem->GetEnabledRegion();
				if(!EnabledRegion.IsEmpty())
				{
					FOnlineSearchSettingsAccelByte::Set(MatchmakingSearchHandle->QuerySettings, SETTING_GAMESESSION_REQUESTEDREGIONS, RegionPreferencesSubsystem->GetEnabledRegion(), EOnlineComparisonOp::In);
				}
			}
		}
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
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnStartMatchmakingComplete(SessionName, FOnlineError(false), {});
		}));
	}
}
// @@@SNIPEND

void UAccelByteWarsOnlineSession::CancelMatchmaking(APlayerController* PC, const FName& SessionName)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

	// safety
	if (!ensure(GetSessionInt()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!ensure(GetABSessionInt()->GetCurrentMatchmakingSearchHandle().IsValid() &&
		GetABSessionInt()->GetCurrentMatchmakingSearchHandle()->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Searching player ID is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCancelMatchmakingComplete(SessionName, false);
		}));
		return;
	}

	if (!GetSessionInt()->CancelMatchmaking(
		*GetABSessionInt()->GetCurrentMatchmakingSearchHandle()->GetSearchingPlayerId(),
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed executing"))
		// Failed to start matchmaking.
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
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
		TEXT("Succeeded: %s | error: (%s) %s"),
		*FString(ErrorDetails.bSucceeded ? "TRUE": "FALSE"),
		*ErrorDetails.ErrorCode, *ErrorDetails.ErrorMessage.ToString())

	OnStartMatchmakingCompleteDelegates.Broadcast(SessionName, ErrorDetails.bSucceeded);
}

void UAccelByteWarsOnlineSession::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCancelMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnMatchmakingComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	const TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle = GetABSessionInt()->GetCurrentMatchmakingSearchHandle();
	if (!bSucceeded ||
		!ensure(CurrentMatchmakingSearchHandle.IsValid()) /*This might happen when the MM finished just as we are about to cancel it*/ ||
		!ensure(CurrentMatchmakingSearchHandle->SearchResults.IsValidIndex(0)) ||
		!ensure(CurrentMatchmakingSearchHandle->GetSearchingPlayerId().IsValid()))
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("There is no match result returned."));
		OnMatchmakingCompleteDelegates.Broadcast(SessionName, false);
		return;
	}

	OnMatchmakingCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnBackfillProposalReceived(
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

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
		UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s To accept backfill."), *FString(bSucceeded ? "TRUE": "FALSE"));
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
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

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
	
	// Get match session template based on game mode type
	FString MatchTemplateName = MatchSessionTemplateNameMap[{NetworkType, GameModeType}];
	
	// if not using AMS, remove suffix -ams (internal purpose)
	if(NetworkType == EGameModeNetworkType::DS && !UTutorialModuleOnlineUtility::GetIsServerUseAMS())
	{
		MatchTemplateName = MatchTemplateName.Replace(TEXT("-ams"), TEXT(""));
	}

	// Override match session template name if applicable.
	if (NetworkType == EGameModeNetworkType::DS && !UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride();
	}
	else if (NetworkType == EGameModeNetworkType::P2P && !UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride();
	}

	// include region preferences into session setting
	if(NetworkType == EGameModeNetworkType::DS)
	{
		if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
			FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
			this,
			true))
		{
			if (!ModuleDataAsset->IsStarterModeActive())
			{
				UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
				ensure(GameInstance);
	
				URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					TArray<FString> EnabledRegion = RegionPreferencesSubsystem->GetEnabledRegion();
					if(!EnabledRegion.IsEmpty())
					{
						FOnlineSessionSettingsAccelByte::Set(SessionSettings, SETTING_GAMESESSION_REQUESTEDREGIONS, RegionPreferencesSubsystem->GetEnabledRegion());
					}
				}
			}
		}
	}
	
	CreateSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchTemplateName);
}

void UAccelByteWarsOnlineSession::FindSessions(
	const int32 LocalUserNum,
	const int32 MaxQueryNum,
	const bool bForce)
{
	UE_LOG_ONLINESESSION(Verbose, TEXT("Called"))

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

void UAccelByteWarsOnlineSession::OnFindSessionsComplete(bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

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

		// remove session that not in region preferences
		if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
			FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
			this,
			true))
		{
			if (!ModuleDataAsset->IsStarterModeActive())
			{				
				UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
				ensure(GameInstance);
	
				URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					RegionPreferencesSubsystem->FilterSessionSearch(SessionSearch);
				}
			}
		}

		// get owners user info for query user info
		TArray<FUniqueNetIdRef> UserIds;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UserIds.AddUnique(SearchResult.Session.OwningUserId->AsShared());
		}

		// trigger Query User info
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

void UAccelByteWarsOnlineSession::OnQueryUserInfoForFindSessionComplete(
	const FOnlineError& Error,
	const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
{
	UE_LOG_ONLINESESSION(Log, TEXT("Succeeded: %s"), *FString(Error.bSucceeded ? "TRUE": "FALSE"))

	if (Error.bSucceeded)
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

#pragma region "Party Essentials"
UPromptSubsystem* UAccelByteWarsOnlineSession::GetPromptSubystem()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}

void UAccelByteWarsOnlineSession::OnCreatePartyToInviteMember(FName SessionName, bool bWasSuccessful, const int32 LocalUserNum, const FUniqueNetIdPtr SenderId, const FUniqueNetIdPtr InviteeId)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	GetOnCreateSessionCompleteDelegates()->Remove(OnCreatePartyToInviteMemberDelegateHandle);

	if (!bWasSuccessful)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot send a party invitation. Failed to create a new party."));
		OnSendPartyInviteComplete(SenderId.ToSharedRef().Get(), SessionName, false, InviteeId.ToSharedRef().Get());
	}
	else
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Party created. Try sending a party invitation."));
		SendPartyInvite(LocalUserNum, InviteeId);
	}
}

void UAccelByteWarsOnlineSession::OnLeavePartyToTriggerEvent(FName SessionName, bool bSucceeded, const TDelegate<void(bool bWasSuccessful)> OnComplete)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	OnComplete.ExecuteIfBound(bSucceeded);
}

void UAccelByteWarsOnlineSession::InitializePartyGeneratedWidgets()
{
	// Assign action button to invite player to the party.
	InviteToPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_to_party"));
	if (ensure(InviteToPartyButtonMetadata))
	{
		InviteToPartyButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
		{
			const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
			if (!ParentWidget)
			{
				return;
			}

			const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
			if (FriendUserId)
			{
				OnInviteToPartyButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
			}
		});
		InviteToPartyButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
	}

	// Assign action button to kick player from the party.
	KickPlayerFromPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_kick_from_party"));
	if (ensure(KickPlayerFromPartyButtonMetadata))
	{
		KickPlayerFromPartyButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
		{
			const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
			if (!ParentWidget)
			{
				return;
			}

			const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
			if (FriendUserId)
			{
				OnKickPlayerFromPartyButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
			}
		});
		KickPlayerFromPartyButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
	}

	// Assign action button to promote party leader.
	PromotePartyLeaderButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_promote_party_leader"));
	if (ensure(PromotePartyLeaderButtonMetadata))
	{
		PromotePartyLeaderButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
		{
			const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
			if (!ParentWidget)
			{
				return;
			}

			const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
			if (FriendUserId)
			{
				OnPromotePartyLeaderButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
			}
		});
		PromotePartyLeaderButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
	}

	// On party update events, update the generated widget.
	if (GetOnCreatePartyCompleteDelegates())
	{
		GetOnCreatePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
	if (GetOnLeavePartyCompleteDelegates())
	{
		GetOnLeavePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
	if (GetOnPartyMembersChangeDelegates())
	{
		GetOnPartyMembersChangeDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member, bool bJoined)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
	if (GetOnPartySessionUpdateReceivedDelegates())
	{
		GetOnPartySessionUpdateReceivedDelegates()->AddWeakLambda(this, [this](FName SessionName)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
}

void UAccelByteWarsOnlineSession::UpdatePartyGeneratedWidgets()
{
	// Take local user id reference from active widget.
	FUniqueNetIdPtr LocalUserABId = nullptr;
	if (UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this))
	{
		LocalUserABId = GetLocalPlayerUniqueNetId(ActiveWidget->GetOwningPlayer());
	}

	// Take current displayed friend id.
	const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();

	// Check party information.
	const bool bIsInParty = IsInParty(LocalUserABId);
	const bool bIsLeader = IsPartyLeader(LocalUserABId);
	const bool bIsFriendInParty = IsInParty(FriendUserId);

	// Display invite to party button if in a party.
	if (InviteToPartyButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(InviteToPartyButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(true);
			Button->SetVisibility(
				!bIsFriendInParty ?
				ESlateVisibility::Visible :
				ESlateVisibility::Collapsed);
		}
	}

	// Display promote leader button if in a party and is the party leader.
	if (PromotePartyLeaderButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(PromotePartyLeaderButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(true);
			Button->SetVisibility(
				(bIsInParty && bIsFriendInParty && bIsLeader) ?
				ESlateVisibility::Visible :
				ESlateVisibility::Collapsed);
		}
	}

	// Display kick player button if in a party and is the party leader.
	if (KickPlayerFromPartyButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(KickPlayerFromPartyButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(true);
			Button->SetVisibility(
				(bIsInParty && bIsFriendInParty && bIsLeader) ?
				ESlateVisibility::Visible :
				ESlateVisibility::Collapsed);
		}
	}
}

void UAccelByteWarsOnlineSession::DeinitializePartyGeneratedWidgets()
{
	// Unbind party action button delegates.
	if (InviteToPartyButtonMetadata)
	{
		InviteToPartyButtonMetadata->ButtonAction.RemoveAll(this);
		InviteToPartyButtonMetadata->OnWidgetGenerated.RemoveAll(this);
	}
	if (KickPlayerFromPartyButtonMetadata)
	{
		KickPlayerFromPartyButtonMetadata->ButtonAction.RemoveAll(this);
		KickPlayerFromPartyButtonMetadata->OnWidgetGenerated.RemoveAll(this);
	}
	if (PromotePartyLeaderButtonMetadata)
	{
		PromotePartyLeaderButtonMetadata->ButtonAction.RemoveAll(this);
		PromotePartyLeaderButtonMetadata->OnWidgetGenerated.RemoveAll(this);
	}

	// Unbind party event delegates.
	if (GetOnPartyMembersChangeDelegates())
	{
		GetOnPartyMembersChangeDelegates()->RemoveAll(this);
	}
	if (GetOnPartySessionUpdateReceivedDelegates())
	{
		GetOnPartySessionUpdateReceivedDelegates()->RemoveAll(this);
	}
}

FUniqueNetIdPtr UAccelByteWarsOnlineSession::GetCurrentDisplayedFriendId()
{
	UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (!ParentWidget)
	{
		return nullptr;
	}

	FUniqueNetIdRepl FriendUserId = nullptr;
	if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
	{
		if (FriendDetailsWidget->GetCachedFriendData() &&
			FriendDetailsWidget->GetCachedFriendData()->UserId &&
			FriendDetailsWidget->GetCachedFriendData()->UserId.IsValid())
		{
			FriendUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
		}
	}

	if (FriendUserId == nullptr || !FriendUserId.IsValid())
	{
		return nullptr;
	}

	return FriendUserId.GetUniqueNetId();
}

void UAccelByteWarsOnlineSession::OnInviteToPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee)
{
	// Disable the button to avoid spamming.
	if (InviteToPartyButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(InviteToPartyButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(false);
		}
	}

	SendPartyInvite(LocalUserNum, Invitee);
}

void UAccelByteWarsOnlineSession::OnKickPlayerFromPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer)
{
	// Disable the button to avoid spamming.
	if (KickPlayerFromPartyButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(KickPlayerFromPartyButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(false);
		}
	}

	KickPlayerFromParty(LocalUserNum, KickedPlayer);
}

void UAccelByteWarsOnlineSession::OnPromotePartyLeaderButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader)
{
	// Disable the button to avoid spamming.
	if (PromotePartyLeaderButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(PromotePartyLeaderButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(false);
		}
	}

	PromotePartyLeader(LocalUserNum, NewLeader);
}

TArray<FUniqueNetIdRef> UAccelByteWarsOnlineSession::GetPartyMembers()
{
	if (GetABSessionInt())
	{
		const FNamedOnlineSession* PartySession = GetABSessionInt()->GetNamedSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession));
		if (PartySession)
		{
			return PartySession->RegisteredPlayers;
		}
	}

	return TArray<FUniqueNetIdRef>();
}

FUniqueNetIdPtr UAccelByteWarsOnlineSession::GetPartyLeader()
{
	if (GetABSessionInt())
	{
		const FNamedOnlineSession* PartySession = GetABSessionInt()->GetNamedSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession));
		if (PartySession)
		{
			const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(PartySession->SessionInfo);
			if (!SessionInfo)
			{
				return nullptr;
			}

			return GetABSessionInt()->GetSessionLeaderId(PartySession);
		}
	}

	return nullptr;
}

bool UAccelByteWarsOnlineSession::IsInParty(const FUniqueNetIdPtr UserId)
{
	if (!UserId)
	{
		return false;
	}

	const TPartyMemberArray Members = GetPartyMembers();
	for (const auto& Member : Members)
	{
		if (!Member.Get().IsValid())
		{
			continue;
		}

		if (Member.Get() == UserId.ToSharedRef().Get())
		{
			return true;
		}
	}

	return false;
}

bool UAccelByteWarsOnlineSession::IsPartyLeader(const FUniqueNetIdPtr UserId)
{
	return GetPartyLeader() && UserId && UserId.ToSharedRef().Get() == GetPartyLeader().ToSharedRef().Get();
}

void UAccelByteWarsOnlineSession::CreateParty(const int32 LocalUserNum)
{
	const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

	// Safety.
	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot create a party. Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreatePartyComplete(SessionName, false);
		}));
		return;
	}

	// Always create a new party. Thus, leave any left-over party session first.
	if (GetABSessionInt()->IsInPartySession())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Party found. Leave old party before creating a new one."));

		if (OnLeaveSessionForTriggerDelegateHandle.IsValid())
		{
			GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);
			OnLeaveSessionForTriggerDelegateHandle.Reset();
		}

		OnLeaveSessionForTriggerDelegateHandle = GetOnLeaveSessionCompleteDelegates()->AddUObject(
			this,
			&ThisClass::OnLeavePartyToTriggerEvent,
			TDelegate<void(bool)>::CreateWeakLambda(this, [this, LocalUserNum, SessionName](bool bWasSuccessful)
			{
				GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);

				if (bWasSuccessful)
				{
					UE_LOG_ONLINESESSION(Log, TEXT("Success to leave old party destroyed. Try creating a new party."));
					CreateParty(LocalUserNum);
				}
				else
				{
					UE_LOG_ONLINESESSION(Warning, TEXT("Cannot create a new party. Failed to leave old party."));
					ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
					{
						OnCreatePartyComplete(SessionName, false);
					}));
				}
			}
		));

		LeaveSession(SessionName);
		return;
	}

	// Create a new party session. Override party session template name if applicable.
	UE_LOG_ONLINESESSION(Log, TEXT("Create a new party."));
	CreateSession(
		LocalUserNum,
		SessionName,
		FOnlineSessionSettings(),
		EAccelByteV2SessionType::PartySession,
		UTutorialModuleOnlineUtility::GetPartySessionTemplateOverride().IsEmpty() ?
			PartySessionTemplate : UTutorialModuleOnlineUtility::GetPartySessionTemplateOverride());
}

void UAccelByteWarsOnlineSession::LeaveParty(const int32 LocalUserNum)
{
	const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot leave a party. Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeavePartyComplete(SessionName, false);
		}));
		return;
	}

	if (!GetABSessionInt()->IsInPartySession()) 
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot leave a party. Not in any party."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeavePartyComplete(SessionName, false);
		}));
		return;
	}

	// Leave party.
	UE_LOG_ONLINESESSION(Log, TEXT("Leave party."));
	LeaveSession(SessionName);
}

void UAccelByteWarsOnlineSession::SendPartyInvite(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee)
{
	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot send a party invitation. Session Interface is not valid."));
		return;
	}

	const APlayerController* SenderPC = GetPlayerControllerByLocalUserNum(LocalUserNum);
	if (!SenderPC)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot send a party invitation. Sender's PlayerController is not valid."));
		return;
	}

	const FUniqueNetIdPtr SenderId = GetLocalPlayerUniqueNetId(SenderPC);
	if (!SenderId)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot send a party invitation. Sender's NetId is not valid."));
		return;
	}

	const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);
	if (!Invitee)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot send a party invitation. Invitee's NetId is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SenderId, SessionName, Invitee]()
		{
			OnSendPartyInviteComplete(SenderId.ToSharedRef().Get(), SessionName, false, Invitee.ToSharedRef().Get());
		}));
		return;
	}

	// Create a new party first before inviting.
	if (!GetABSessionInt()->IsInPartySession())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Not in a party session. Creating a new party before sending a party invitation."));

		if (OnCreatePartyToInviteMemberDelegateHandle.IsValid())
		{
			GetOnCreateSessionCompleteDelegates()->Remove(OnCreatePartyToInviteMemberDelegateHandle);
			OnCreatePartyToInviteMemberDelegateHandle.Reset();
		}

		OnCreatePartyToInviteMemberDelegateHandle = GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreatePartyToInviteMember, LocalUserNum, SenderId, Invitee);

		CreateParty(LocalUserNum);
		return;
	}

	// Send party invitation.
	UE_LOG_ONLINESESSION(Log, TEXT("Send party invitation."));
	GetABSessionInt()->SendSessionInviteToFriend(
		SenderId.ToSharedRef().Get(),
		SessionName,
		Invitee.ToSharedRef().Get());
}

void UAccelByteWarsOnlineSession::JoinParty(const int32 LocalUserNum, const FOnlineSessionSearchResult& PartySessionResult)
{
	const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot join a party. Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnJoinPartyComplete(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
		}));
		return;
	}

	// Always leave any party before joining a new party.
	if (GetABSessionInt()->IsInPartySession())
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Party found. Leave old party before joining a new one."));

		if (OnLeaveSessionForTriggerDelegateHandle.IsValid())
		{
			GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);
			OnLeaveSessionForTriggerDelegateHandle.Reset();
		}

		OnLeaveSessionForTriggerDelegateHandle = GetOnLeaveSessionCompleteDelegates()->AddUObject(
			this,
			&ThisClass::OnLeavePartyToTriggerEvent,
			TDelegate<void(bool)>::CreateWeakLambda(this, [this, LocalUserNum, PartySessionResult, SessionName](bool bWasSuccessful)
			{
				GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);

				if (bWasSuccessful)
				{
					UE_LOG_ONLINESESSION(Log, TEXT("Success to leave old party. Try to joining a new party."));

					JoinParty(LocalUserNum, PartySessionResult);
				}
				else
				{
					UE_LOG_ONLINESESSION(Warning, TEXT("Cannot joining a new party. Failed to leave old party."));

					ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
					{
						OnJoinPartyComplete(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
					}));
				}
			}
		));

		LeaveSession(SessionName);
		return;
	}

	// Join a new party.
	UE_LOG_ONLINESESSION(Log, TEXT("Join a new party."));
	JoinSession(LocalUserNum, SessionName, PartySessionResult);
}

void UAccelByteWarsOnlineSession::RejectPartyInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite)
{
	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Session Interface is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnRejectPartyInviteComplete(false);
		}));
		return;
	}

	const APlayerController* RejecterPC = GetPlayerControllerByLocalUserNum(LocalUserNum);
	if (!RejecterPC)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Rejecter's PlayerController is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnRejectPartyInviteComplete(false);
		}));
		return;
	}

	const FUniqueNetIdPtr RejecterId = GetLocalPlayerUniqueNetId(RejecterPC);
	if (!RejecterId)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Rejecter's NetId is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnRejectPartyInviteComplete(false);
		}));
		return;
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Reject party invitation."));
	GetABSessionInt()->RejectInvite(
		RejecterId.ToSharedRef().Get(),
		PartyInvite,
		FOnRejectSessionInviteComplete::CreateUObject(this, &ThisClass::OnRejectPartyInviteComplete));
}

void UAccelByteWarsOnlineSession::KickPlayerFromParty(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer)
{
	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot kick a player from the party. Session Interface is not valid."));
		return;
	}

	if (!KickedPlayer)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot kick a player from the party. KickedPlayer's NetId is not valid."));
		return;
	}

	const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
	if (!PC)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot kick a player from the party. Kicker's PlayerController is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
		{
			OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
		}));
		return;
	}

	const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
	if (!PlayerNetId)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot kick a player from the party. Kicker's NetId is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
		{
			OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
		}));
		return;
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Kick party member."));
	GetABSessionInt()->KickPlayer(
		PlayerNetId.ToSharedRef().Get(),
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession),
		KickedPlayer.ToSharedRef().Get(),
		FOnKickPlayerComplete::CreateUObject(this, &ThisClass::OnKickPlayerFromPartyComplete));
}

void UAccelByteWarsOnlineSession::PromotePartyLeader(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader)
{
	if (!GetABSessionInt())
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot promote new party leader. Session Interface is not valid."));
		return;
	}

	if (!NewLeader)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot promote new party leader. New Leader NetId is not valid."));
		return;
	}

	const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
	if (!PC)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot promote new party leader. Promoter's PlayerController is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, NewLeader]()
		{
			OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
		}));
		return;
	}

	const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
	if (!PlayerNetId)
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Cannot promote new party leader. Promoter's NetId is not valid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, NewLeader]()
		{
			OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
		}));
		return;
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Promote a new party leader."));
	GetABSessionInt()->PromotePartySessionLeader(
		PlayerNetId.ToSharedRef().Get(),
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession),
		NewLeader.ToSharedRef().Get(),
		FOnPromotePartySessionLeaderComplete::CreateUObject(this, &ThisClass::OnPromotePartyLeaderComplete));
}

void UAccelByteWarsOnlineSession::OnCreatePartyComplete(FName SessionName, bool bSucceeded)
{
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	if (bSucceeded)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to create a party"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to create a party"));
	}

	// Cache the party leader.
	LastPartyLeader = GetPartyLeader();

	// Reset the party member status cache.
	PartyMemberStatus.Empty();

	OnCreatePartyCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnLeavePartyComplete(FName SessionName, bool bSucceeded)
{
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	if (bSucceeded)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to leave a party"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave a party"));
	}

	OnLeavePartyCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UAccelByteWarsOnlineSession::OnSendPartyInviteComplete(const FUniqueNetId& Sender, FName SessionName, bool bWasSuccessful, const FUniqueNetId& Invitee)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	const FUniqueNetIdAccelByteUserRef InviteeABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Invitee.AsShared());
	if (bWasSuccessful)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to send party invitation to %s"),
			InviteeABId->IsValid() ? *InviteeABId->GetAccelByteId() : TEXT("Unknown"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to send party invitation to %s"),
			InviteeABId->IsValid() ? *InviteeABId->GetAccelByteId() : TEXT("Unknown"));
	}

	// Display push notification.
	if (GetPromptSubystem())
	{
		GetPromptSubystem()->PushNotification(bWasSuccessful ? SUCCESS_SEND_PARTY_INVITE : FAILED_SEND_PARTY_INVITE);
	}

	UpdatePartyGeneratedWidgets();

	OnSendPartyInviteCompleteDelegates.Broadcast(Sender, SessionName, bWasSuccessful, Invitee);
}

void UAccelByteWarsOnlineSession::OnJoinPartyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to join a party"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to join a party"));
	}

	// Cache the party leader.
	LastPartyLeader = GetPartyLeader();

	// Reset the party member status cache.
	PartyMemberStatus.Empty();

	OnJoinPartyCompleteDelegates.Broadcast(SessionName, Result);
}

void UAccelByteWarsOnlineSession::OnRejectPartyInviteComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to reject party invitation"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to reject party invitation"));
	}

	OnRejectPartyInviteCompleteDelegate.ExecuteIfBound(bWasSuccessful);
	OnRejectPartyInviteCompleteDelegate.Unbind();
}

void UAccelByteWarsOnlineSession::OnPartyInviteRejected(FName SessionName, const FUniqueNetId& RejecterId)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	const FUniqueNetIdAccelByteUserRef RejecterABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(RejecterId.AsShared());
	UE_LOG_ONLINESESSION(Log, TEXT("Party invitation is rejected by %s"),
		RejecterABId->IsValid() ? *RejecterABId->GetAccelByteId() : TEXT("Unknown"));

	// Display push notification to show who rejected the invitation.
	if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
	{
		StartupSubsystem->QueryUserInfo(
			0,
			TPartyMemberArray{ RejecterABId },
			FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [RejecterABId, this](
				const FOnlineError& Error,
				const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
			{
				if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
				{
					return;
				}

				FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

				const FText NotifMessage = FText::Format(PARTY_INVITE_REJECTED_MESSAGE, FText::FromString(
					MemberInfo.GetDisplayName().IsEmpty() ?
					UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(RejecterABId.Get()) :
					MemberInfo.GetDisplayName()
				));

				FString AvatarURL;
				MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

				GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
			}));
	}

	OnPartyInviteRejectedDelegates.Broadcast(SessionName, RejecterId);
}

void UAccelByteWarsOnlineSession::OnPartyInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& PartyInvite)
{
	// Abort if not a party session.
	if (UserId == FromId || PartyInvite.SessionType != EAccelByteV2SessionType::PartySession)
	{
		return;
	}

	const APlayerController* PC = GetPlayerControllerByUniqueNetId(UserId.AsShared());
	if (!PC)
	{
		return;
	}

	const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(FromId.AsShared());
	UE_LOG_ONLINESESSION(Log, TEXT("Receives party invitation from %s"),
		SenderABId->IsValid() ? *SenderABId->GetAccelByteId() : TEXT("Unknown"));

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

	// Display push notification to allow player to accept/reject the party invitation.
	if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
	{
		StartupSubsystem->QueryUserInfo(
			0,
			TPartyMemberArray{SenderABId},
			FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, SenderABId, PartyInvite, LocalUserNum](
				const FOnlineError& Error,
				const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
			{
				if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
				{
					return;
				}

				FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

				const FText NotifMessage = FText::Format(PARTY_INVITE_RECEIVED_MESSAGE, FText::FromString(
					MemberInfo.GetDisplayName().IsEmpty() ?
					UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(SenderABId.Get()) :
					MemberInfo.GetDisplayName()
				));

				FString AvatarURL;
				MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

				GetPromptSubystem()->PushNotification(
					NotifMessage,
					AvatarURL,
					true,
					ACCEPT_PARTY_INVITE_MESSAGE,
					REJECT_PARTY_INVITE_MESSAGE,
					FText::GetEmpty(),
					FPushNotificationDelegate::CreateWeakLambda(this, [this, LocalUserNum, PartyInvite](EPushNotificationActionResult ActionButtonResult)
						{
							switch (ActionButtonResult)
							{
								// Show accept party invitation confirmation.
							case EPushNotificationActionResult::Button1:
								DisplayJoinPartyConfirmation(LocalUserNum, PartyInvite);
								break;
								// Reject party invitation.
							case EPushNotificationActionResult::Button2:
								RejectPartyInvite(LocalUserNum, PartyInvite);
								break;
							}
						}
				));
			}));
	}

	OnPartyInviteReceivedDelegate.ExecuteIfBound(UserId, FromId, PartyInvite);
	OnPartyInviteReceivedDelegate.Unbind();
}

void UAccelByteWarsOnlineSession::DisplayJoinPartyConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite)
{
	// Join the party if not in any party yet.
	if (!GetABSessionInt()->IsInPartySession() || GetPartyMembers().Num() <= 1)
	{
		JoinParty(LocalUserNum, PartyInvite.Session);
		return;
	}

	// Show confirmation to leave current party and join the new party.
	GetPromptSubystem()->ShowDialoguePopUp(
		PARTY_POPUP_MESSAGE,
		JOIN_NEW_PARTY_CONFIRMATION_MESSAGE,
		EPopUpType::ConfirmationYesNo,
		FPopUpResultDelegate::CreateWeakLambda(this, [this, LocalUserNum, PartyInvite](EPopUpResult Result)
		{
			switch (Result)
			{
			case EPopUpResult::Confirmed:
				// If confirmed, join the new party.
				JoinParty(LocalUserNum, PartyInvite.Session);
				break;
			case EPopUpResult::Declined:
				// If declined, reject the party invitation.
				RejectPartyInvite(LocalUserNum, PartyInvite);
				break;
			}
		}
	));
}

void UAccelByteWarsOnlineSession::OnKickPlayerFromPartyComplete(bool bWasSuccessful, const FUniqueNetId& KickedPlayer)
{
	const FUniqueNetIdAccelByteUserRef KickedPlayerABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(KickedPlayer.AsShared());
	if (bWasSuccessful)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to kick %s from the party."),
			KickedPlayerABId->IsValid() ? *KickedPlayerABId->GetAccelByteId() : TEXT("Unknown"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to kick %s from the party."),
			KickedPlayerABId->IsValid() ? *KickedPlayerABId->GetAccelByteId() : TEXT("Unknown"));
	}

	OnKickPlayerFromPartyCompleteDelegate.ExecuteIfBound(bWasSuccessful, KickedPlayer);
	OnKickPlayerFromPartyCompleteDelegate.Unbind();
}

void UAccelByteWarsOnlineSession::OnKickedFromParty(FName SessionName)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Current logged player is kicked from the party"));

	// Display push notification.
	if (GetPromptSubystem())
	{
		GetPromptSubystem()->PushNotification(KICKED_FROM_PARTY_MESSAGE);
	}

	OnKickedFromPartyDelegates.Broadcast(SessionName);
}

void UAccelByteWarsOnlineSession::OnPromotePartyLeaderComplete(const FUniqueNetId& NewLeader, const FOnlineError& Result)
{
	const FUniqueNetIdAccelByteUserRef NewLeaderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(NewLeader.AsShared());
	if (Result.bSucceeded)
	{
		UE_LOG_ONLINESESSION(Log, TEXT("Success to promote %s as the new party leader."),
			NewLeaderABId->IsValid() ? *NewLeaderABId->GetAccelByteId() : TEXT("Unknown"));
	}
	else
	{
		UE_LOG_ONLINESESSION(Warning, TEXT("Failed to promote %s as the new party leader."),
			NewLeaderABId->IsValid() ? *NewLeaderABId->GetAccelByteId() : TEXT("Unknown"));
	}

	OnPromotePartyLeaderCompleteDelegate.ExecuteIfBound(NewLeader, Result);
	OnPromotePartyLeaderCompleteDelegate.Unbind();
}

void UAccelByteWarsOnlineSession::DisplayCurrentPartyLeader()
{
	// Abort if the party leader is the same.
	if (LastPartyLeader && IsPartyLeader(LastPartyLeader))
	{
		return;
	}

	LastPartyLeader = GetPartyLeader();
	const FUniqueNetIdAccelByteUserPtr LeaderABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LastPartyLeader);

	// Query party leader information and then display a notification.
	if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
	{
		StartupSubsystem->QueryUserInfo(
			0,
			TPartyMemberArray{ LeaderABId.ToSharedRef() },
			FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, LeaderABId](
				const FOnlineError& Error,
				const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
			{
				if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
				{
					return;
				}

				FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

				const FText NotifMessage = FText::Format(PARTY_NEW_LEADER_MESSAGE, FText::FromString(
					MemberInfo.GetDisplayName().IsEmpty() ?
					UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(LeaderABId.ToSharedRef().Get()) :
					MemberInfo.GetDisplayName()
				));

				FString AvatarURL;
				MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

				GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
			}));
	}
}

void UAccelByteWarsOnlineSession::OnPartyMembersChange(FName SessionName, const FUniqueNetId& Member, bool bJoined)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	// Store status whether the member is the current logged-in player.
	FUniqueNetIdPtr UserId = nullptr;
	if (GetIdentityInt())
	{
		UserId = GetIdentityInt()->GetUniquePlayerId(0);
	}
	const bool bIsMemberTheLoggedInPlayer = UserId && UserId.ToSharedRef().Get() == Member;

	const FUniqueNetIdAccelByteUserRef MemberABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member.AsShared());
	const FString MemberABIdStr = MemberABId->GetAccelByteId();

	/* Since this event could be called multiple times, we cache the party member status.
	 * This cache is used to execute the following functionalities only when the party member status is changed (not the same status).*/
	if (PartyMemberStatus.Contains(MemberABIdStr))
	{
		if (PartyMemberStatus[MemberABIdStr] == bJoined)
		{
			// Abort if the status is the same.
			return;
		}
		PartyMemberStatus[MemberABIdStr] = bJoined;
	}
	if (!PartyMemberStatus.Contains(MemberABIdStr))
	{
		PartyMemberStatus.Add(MemberABIdStr, bJoined);
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Party participant %s %s to/from the party"),
		MemberABId->IsValid() ? *MemberABId->GetAccelByteId() : TEXT("Unknown"),
		bJoined ? TEXT("joined") : TEXT("left"));

	// Query member information then display a push notification to show who joined/left the party.
	if (!bIsMemberTheLoggedInPlayer)
	{
		if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
		{
			StartupSubsystem->QueryUserInfo(
				0,
				TPartyMemberArray{ MemberABId },
				FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, MemberABId, bJoined](
					const FOnlineError& Error,
					const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
				{
					if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
					{
						return;
					}

					FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

					const FString MemberDisplayName = MemberInfo.GetDisplayName().IsEmpty() ?
						UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(MemberABId.Get()) :
						MemberInfo.GetDisplayName();

					const FText NotifMessage = bJoined ?
						FText::Format(PARTY_MEMBER_JOINED_MESSAGE, FText::FromString(MemberDisplayName)) :
						FText::Format(PARTY_MEMBER_LEFT_MESSAGE, FText::FromString(MemberDisplayName));

					FString AvatarURL;
					MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

					GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
				}));
		}
	}

	// Show notification if a new party leader is set.
	DisplayCurrentPartyLeader();

	OnPartyMembersChangeDelegates.Broadcast(SessionName, Member, bJoined);
}

void UAccelByteWarsOnlineSession::OnPartySessionUpdateReceived(FName SessionName)
{
	// Abort if not a party session.
	if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
	{
		return;
	}

	UE_LOG_ONLINESESSION(Log, TEXT("Party session is updated"));

	// Show notification if a new party leader is set.
	DisplayCurrentPartyLeader();

	OnPartySessionUpdateReceivedDelegates.Broadcast(SessionName);
}

#pragma endregion

void UAccelByteWarsOnlineSession::OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ensureMsgf(GameInstance, TEXT("Game Instance is null")))
	{
		return;
	}

	FOnlineIdentityAccelBytePtr ABIdentityInt = GetABIdentityInt();
	if (!ensureMsgf(ABIdentityInt, TEXT("AB OnlineIdentity interface is nullptr."))) 
	{
		return;
	}

	FOnlineSessionV2AccelBytePtr ABSessionInt = GetABSessionInt();
	if (!ensureMsgf(ABSessionInt, TEXT("AB OnlineSession interface is null.")))
	{
		return;
	}

	if (!bSucceeded)
	{
		// Show failed to reconnect to AGS pop-up message.
		if (Error.Equals(LOBBY_CONNECT_ERROR_CODE) && GameInstance->bIsReconnecting)
		{
			GetPromptSubystem()->ShowMessagePopUp(ERROR_PROMPT_TEXT, LOBBY_FAILED_RECONNECT_MESSAGE);
		}
		GameInstance->bIsReconnecting = false;
		return;
	}

	if (GameInstance->bIsReconnecting)
	{
		// Show success to reconnect to AGS pop-up message.
		GameInstance->bIsReconnecting = false;
		GetPromptSubystem()->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, LOBBY_SUCCESS_RECONNECT_MESSAGE);
	}

	// Restore and leave old party session.
	ABSessionInt->RestoreActiveSessions(
		UserId,
		FOnRestoreActiveSessionsComplete::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlineError& Result)
		{
			// Abort if failed to restore party sessions.
			if (!Result.bSucceeded)
			{
				UE_LOG_ONLINESESSION(Warning, TEXT("Failed to restore party session. Error: %s"), *Result.ErrorMessage.ToString());
				return;
			}

			// Safety.
			if (!GetABSessionInt())
			{
				UE_LOG_ONLINESESSION(Warning, TEXT("Failed to restore party session. Session Interface is not valid."));
				return;
			}

			const TArray<FOnlineRestoredSessionAccelByte> RestoredParties = GetABSessionInt()->GetAllRestoredPartySessions();

			// If no restored party session, do nothing.
			if (RestoredParties.IsEmpty())
			{
				UE_LOG_ONLINESESSION(Log, TEXT("No restored party session found. Do nothing."));
				return;
			}

			// Leave the first restored active party session.
			UE_LOG_ONLINESESSION(Log, TEXT("Restored party session found. Leave the restored party session."));
			GetABSessionInt()->LeaveRestoredSession(
				LocalUserId, 
				RestoredParties[0],
				FOnLeaveSessionComplete::CreateWeakLambda(this, [](bool bWasSuccessful, FString SessionId)
				{
					if (bWasSuccessful)
					{
						UE_LOG_ONLINESESSION(Log, TEXT("Success to leave restored party session."));
					}
					else
					{
						UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave restored party session."));
					}
				}
			));
		})
	);

	ABIdentityInt->AccelByteOnLobbyReconnectingDelegates->RemoveAll(this);
	ABIdentityInt->AccelByteOnLobbyReconnectedDelegates->RemoveAll(this);
	ABIdentityInt->AccelByteOnLobbyConnectionClosedDelegates->RemoveAll(this);

	ABIdentityInt->AccelByteOnLobbyReconnectingDelegates->AddUObject(this, &ThisClass::OnLobbyReconnecting);
	ABIdentityInt->AccelByteOnLobbyReconnectedDelegates->AddUObject(this, &ThisClass::OnLobbyReconnected);
	ABIdentityInt->AccelByteOnLobbyConnectionClosedDelegates->AddUObject(this, &ThisClass::OnLobbyConnectionClosed);
}

void UAccelByteWarsOnlineSession::OnLobbyReconnecting(int32 LocalUserNum, const FUniqueNetId& UserId, int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ensureMsgf(GameInstance, TEXT("Game Instance is null")))
	{
		return;
	}

	GameInstance->bIsReconnecting = true;
	GetPromptSubystem()->ShowLoading(LOBBY_RECONNECTING_MESSAGE);
}

void UAccelByteWarsOnlineSession::OnLobbyReconnected()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ensureMsgf(GameInstance, TEXT("Game Instance is null")))
	{
		return;
	}

	GameInstance->bIsReconnecting = false;
	GetPromptSubystem()->HideLoading();
}

void UAccelByteWarsOnlineSession::OnLobbyConnectionClosed(int32 LocalUserNum, const FUniqueNetId& UserId, int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ensureMsgf(GameInstance, TEXT("Game Instance is null")))
	{
		return;
	}

	GameInstance->bIsReconnecting = false;
	GetPromptSubystem()->HideLoading();

	if (StatusCode == static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectFromExternalReconnect))
	{
		// Do some manual handle to reconnect lobby
		FOnlineIdentityAccelBytePtr ABIdentityInt = GetABIdentityInt();
		if (!ensureMsgf(ABIdentityInt, TEXT("AB OnlineIdentity interface is nullptr."))) 
		{
			return;
		}

		ABIdentityInt->ConnectAccelByteLobby(LocalUserNum);
		
		GameInstance->bIsReconnecting = true;
		GetPromptSubystem()->ShowLoading(LOBBY_RECONNECTING_MESSAGE);
	}
}
