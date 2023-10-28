// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsOnlineSession.h"

#include "AccelByteWarsOnlineSessionLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget_Starter.h"

void UAccelByteWarsOnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// Session Essentials
	GetSessionInt()->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	GetSessionInt()->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	GetABSessionInt()->AddOnSendSessionInviteCompleteDelegate_Handle(
		FOnSendSessionInviteCompleteDelegate::CreateUObject(this, &ThisClass::OnSendSessionInviteComplete));
	GetSessionInt()->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnLeaveSessionComplete));

	GetABSessionInt()->AddOnV2SessionInviteReceivedDelegate_Handle(
		FOnV2SessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived));
	GetABSessionInt()->AddOnSessionParticipantsChangeDelegate_Handle(
		FOnSessionParticipantsChangeDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsChange));

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
    UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// Matchmaking Essentials
	GetSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnMatchmakingComplete);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.AddUObject(this, &ThisClass::OnBackfillProposalReceived);

	// Match Session Essentials
	GetSessionInt()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);
	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;

	// Party Essentials
    GetABSessionInt()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreatePartyComplete);
    GetABSessionInt()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinPartyComplete);
    GetABSessionInt()->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeavePartyComplete);
    GetABSessionInt()->OnSendSessionInviteCompleteDelegates.AddUObject(this, &ThisClass::OnSendPartyInviteComplete);
	GetABSessionInt()->OnSessionInviteRejectedDelegates.AddUObject(this, &ThisClass::OnPartyInviteRejected);
    GetABSessionInt()->OnV2SessionInviteReceivedDelegates.AddUObject(this, &ThisClass::OnPartyInviteReceived);
    GetABSessionInt()->OnKickedFromSessionDelegates.AddUObject(this, &ThisClass::OnKickedFromParty);
    GetABSessionInt()->OnSessionParticipantsChangeDelegates.AddUObject(this, &ThisClass::OnPartyMembersChange);
    GetABSessionInt()->OnSessionUpdateReceivedDelegates.AddUObject(this, &ThisClass::OnPartySessionUpdateReceived);
    GetABIdentityInt()->OnConnectLobbyCompleteDelegates->AddUObject(this, &ThisClass::OnConnectLobbyComplete);
    InitializePartyGeneratedWidgets();
}

void UAccelByteWarsOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	// Session Essentials
	GetSessionInt()->ClearOnCreateSessionCompleteDelegates(this);
	GetSessionInt()->ClearOnJoinSessionCompleteDelegates(this);
	GetABSessionInt()->ClearOnSendSessionInviteCompleteDelegates(this);
	GetSessionInt()->ClearOnDestroySessionCompleteDelegates(this);

	GetABSessionInt()->ClearOnV2SessionInviteReceivedDelegates(this);
	GetABSessionInt()->ClearOnSessionParticipantsChangeDelegates(this);

	// Game Session Essentials
	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
    UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	// Matchmaking Essentials
	GetSessionInt()->OnMatchmakingCompleteDelegates.RemoveAll(this);
	GetSessionInt()->OnCancelMatchmakingCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnBackfillProposalReceivedDelegates.RemoveAll(this);

	// Match Session Essentials
	GetSessionInt()->OnFindSessionsCompleteDelegates.RemoveAll(this);

	// Party Essentials
	GetABSessionInt()->OnCreateSessionCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnJoinSessionCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnDestroySessionCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnSendSessionInviteCompleteDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionInviteRejectedDelegates.RemoveAll(this);
	GetABSessionInt()->OnV2SessionInviteReceivedDelegates.RemoveAll(this);
	GetABSessionInt()->OnKickedFromSessionDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionParticipantsChangeDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionUpdateReceivedDelegates.RemoveAll(this);
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

void UAccelByteWarsOnlineSession::OnSendSessionInviteComplete(
	const FUniqueNetId& LocalSenderId,
	FName SessionName,
	bool bSucceeded,
	const FUniqueNetId& InviteeId)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"))

	OnSendSessionInviteCompleteDelegates.Broadcast(LocalSenderId, SessionName, bSucceeded, InviteeId);
}

void UAccelByteWarsOnlineSession::OnRejectSessionInviteComplete(bool bSucceeded)
{
	UE_LOG_ONLINESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"))

	OnRejectSessionInviteCompleteDelegates.Broadcast(bSucceeded);
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

void UAccelByteWarsOnlineSession::OnSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	UE_LOG_ONLINESESSION(Log, TEXT("from: %s"), *FromId.ToDebugString())

	OnSessionInviteReceivedDelegates.Broadcast(UserId, FromId, Invite);
}

void UAccelByteWarsOnlineSession::OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member,
	bool bJoined)
{
	UE_LOG_ONLINESESSION(
		Log,
		TEXT("session name: %s | Member: %s [%s]"),
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
		
        UE_LOG_ONLINESESSION(Log, 
            TEXT("Queried users info: %d, found valid users info: %d"), 
            UserIds.Num(), OnlineUsers.Num());

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

	// Check is using AMS
	const bool bUseAMS = UTutorialModuleOnlineUtility::GetIsServerUseAMS();
	
	// Get match pool id based on game mode type
	FString MatchPoolId = MatchmakingPoolIdMap[{NetworkType, GameModeType}];

	if(NetworkType == EGameModeNetworkType::DS && bUseAMS)
	{
		MatchPoolId.Append("-ams");
	}
	
	// Setup matchmaking search handle, it will be used to store session search results.
	TSharedRef<FOnlineSessionSearch> MatchmakingSearchHandle = MakeShared<FOnlineSessionSearch>();
	MatchmakingSearchHandle->QuerySettings.Set(
		SETTING_SESSION_MATCHPOOL, MatchPoolId, EOnlineComparisonOp::Equals);

    // Check for DS version override.
    const FString OverriddenDSVersion = UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride();
    if (!OverriddenDSVersion.IsEmpty())
    {
        MatchmakingSearchHandle->QuerySettings.Set(SETTING_GAMESESSION_CLIENTVERSION, OverriddenDSVersion, EOnlineComparisonOp::Equals);
    }

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

	// Check is using AMS
	const bool bUseAMS = UTutorialModuleOnlineUtility::GetIsServerUseAMS();
	
	// Get match pool id based on game mode type
	FString MatchTemplateName = MatchSessionTemplateNameMap[{NetworkType, GameModeType}];
	
	if(NetworkType == EGameModeNetworkType::DS && bUseAMS)
	{
		MatchTemplateName.Append("-ams");
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

    // On party member update events, update the generated widget.
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
    // Abort if not in a party session.
    if (!GetABSessionInt()->IsInPartySession())
    {
        return;
    }

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
                (bIsInParty && !bIsFriendInParty) ?
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
    else if (const UFriendDetailsWidget_Starter* FriendDetailsWidget_Starter = Cast<UFriendDetailsWidget_Starter>(ParentWidget))
    {
        if (FriendDetailsWidget_Starter->GetCachedFriendData() &&
            FriendDetailsWidget_Starter->GetCachedFriendData()->UserId &&
            FriendDetailsWidget_Starter->GetCachedFriendData()->UserId.IsValid())
        {
            FriendUserId = FriendDetailsWidget_Starter->GetCachedFriendData()->UserId;
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
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnCreatePartyComplete(SessionName, false);
        }));
        return;
    }

    // Always create a new party. Thus, leave any left-over party session first.
    if (GetABSessionInt()->IsInPartySession())
    {
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
                if (bWasSuccessful)
                {
                    CreateParty(LocalUserNum);
                }
                else
                {
                    ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
                        {
                            OnCreatePartyComplete(SessionName, false);
                        }));
                }
            }
        ));

        LeaveSession(SessionName);
        return;
    }

    // Create a new party session.
    CreateSession(
        LocalUserNum,
        SessionName,
        FOnlineSessionSettings(),
        EAccelByteV2SessionType::PartySession,
        PartySessionTemplate);
}

void UAccelByteWarsOnlineSession::LeaveParty(const int32 LocalUserNum)
{
    const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

    if (!GetABSessionInt() || !GetABSessionInt()->IsInPartySession())
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot leave a party. Session Interface is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnLeavePartyComplete(SessionName, false);
        }));
        return;
    }

    // After leaving a party, automatically create a new one.
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
            if (bWasSuccessful)
            {
                CreateParty(LocalUserNum);
            }
            else
            {
                ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
                {
                    OnLeavePartyComplete(SessionName, false);
                }));
            }
        }
    ));

    // Leave party.
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
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SenderId, SessionName, Invitee]()
        {
            OnSendPartyInviteComplete(SenderId.ToSharedRef().Get(), SessionName, false, Invitee.ToSharedRef().Get());
        }));
        return;
    }

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
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnJoinPartyComplete(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
        }));
        return;
    }

    // Always leave any party before joining a new party.
    if (GetABSessionInt()->IsInPartySession())
    {
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
                if (bWasSuccessful)
                {
                    JoinParty(LocalUserNum, PartySessionResult);
                }
                else
                {
                    ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, SessionName]()
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
    JoinSession(LocalUserNum, SessionName, PartySessionResult);
}

void UAccelByteWarsOnlineSession::RejectPartyInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite)
{
    if (!GetABSessionInt())
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Session Interface is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

    const APlayerController* RejecterPC = GetPlayerControllerByLocalUserNum(LocalUserNum);
    if (!RejecterPC)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Rejecter's PlayerController is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

    const FUniqueNetIdPtr RejecterId = GetLocalPlayerUniqueNetId(RejecterPC);
    if (!RejecterId)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot reject a party invitation. Rejecter's NetId is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

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
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
        {
            OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
        }));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
    if (!PlayerNetId)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot kick a player from the party. Kicker's NetId is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
        {
            OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
        }));
        return;
    }

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
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, NewLeader]()
        {
            OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
        }));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
    if (!PlayerNetId)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot promote new party leader. Promoter's NetId is not valid."));
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, NewLeader]()
        {
            OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
        }));
        return;
    }

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
    QueryUserInfo(0, TPartyMemberArray{ RejecterABId },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, RejecterABId](const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
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
        }
    ));

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
    QueryUserInfo(0, TPartyMemberArray{ SenderABId },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, LocalUserNum, SenderABId, PartyInvite]
        (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
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
        }
    ));

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
    QueryUserInfo(0, TPartyMemberArray{ LeaderABId.ToSharedRef() },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, LeaderABId]
        (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
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
        }
    ));
}

void UAccelByteWarsOnlineSession::OnPartyMembersChange(FName SessionName, const FUniqueNetId& Member, bool bJoined)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    const FUniqueNetIdAccelByteUserRef MemberABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member.AsShared());
    const FString MemberABUIdStr = MemberABId->GetAccelByteId();

    UE_LOG_ONLINESESSION(Log, TEXT("Party participant %s %s to/from the party"),
        MemberABId->IsValid() ? *MemberABId->GetAccelByteId() : TEXT("Unknown"),
        bJoined ? TEXT("joined") : TEXT("left"));

    /* Since this event could be called multiple times, we cache the party member status.
     * This cache is used to execute the following functionalities only when the party member status is changed (not the same status).*/
    if (PartyMemberStatus.Contains(MemberABUIdStr))
    {
        if (PartyMemberStatus[MemberABUIdStr] == bJoined)
        {
            // Abort if the status is the same.
            return;
        }
        PartyMemberStatus[MemberABUIdStr] = bJoined;
    }
    if (!PartyMemberStatus.Contains(MemberABUIdStr))
    {
        PartyMemberStatus.Add(MemberABUIdStr, bJoined);
    }

    // Query member information then display a push notification to show who joined/left the party.
    QueryUserInfo(0, TPartyMemberArray{ MemberABId },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, MemberABId, bJoined]
        (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
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
        }
    ));

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

void UAccelByteWarsOnlineSession::LeaveRestoredPartyToTriggerEvent(const FUniqueNetId& LocalUserId, const FOnlineError& Result, const TDelegate<void(bool bSucceeded)> OnComplete)
{
    // Abort if failed to restore sessions.
    if (!Result.bSucceeded)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave restored party sessions. Error: %s"), *Result.ErrorMessage.ToString());
        OnComplete.ExecuteIfBound(false);
        return;
    }

    // Safety.
    if (!GetABSessionInt())
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave restored party sessions. Session Interface is not valid."));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    const TArray<FOnlineRestoredSessionAccelByte> RestoredParties = GetABSessionInt()->GetAllRestoredPartySessions();

    // If empty, no need to leave the restored party sessions.
    if (RestoredParties.IsEmpty())
    {
        UE_LOG_ONLINESESSION(Log, TEXT("No need to leave party session, restored party sessions are empty."));
        OnComplete.ExecuteIfBound(true);
        return;
    }

    // Leave the restored party session then invoke the on-complete event.
    // Since player can only be in a single party, then leave the first restored party session.
    GetABSessionInt()->LeaveRestoredSession(
        LocalUserId,
        RestoredParties[0],
        FOnLeaveSessionComplete::CreateUObject(this, &ThisClass::OnLeaveRestoredPartyToTriggerEventComplete, OnComplete));
}

void UAccelByteWarsOnlineSession::OnLeaveRestoredPartyToTriggerEventComplete(bool bSucceeded, FString SessionId, const TDelegate<void(bool bSucceeded)> OnComplete)
{
    if (bSucceeded)
    {
        UE_LOG_ONLINESESSION(Log, TEXT("Success to leave restored party session %s"), *SessionId);
    }
    else
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Failed to leave restored party session %s"), *SessionId);
    }

    OnComplete.ExecuteIfBound(bSucceeded);
}

void UAccelByteWarsOnlineSession::OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error)
{
    if (!bSucceeded)
    {
        UE_LOG_ONLINESESSION(Warning, TEXT("Cannot initialize party. Failed to connect to lobby. Error: %s."), *Error);
        return;
    }

    // Bind event to create a new party when got kicked.
    GetOnKickedFromPartyDelegates()->AddWeakLambda(this, [this, LocalUserNum](FName SessionName)
    {
        if (SessionName.IsEqual(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))) 
        {
            UE_LOG_ONLINESESSION(Log, TEXT("Creating new party after got kicked from the last party"));
            CreateParty(LocalUserNum);
        }
    });

    // Restore and leave party, then create a new party.
    GetABSessionInt()->RestoreActiveSessions(
        UserId,
        FOnRestoreActiveSessionsComplete::CreateUObject(
            this,
            &ThisClass::LeaveRestoredPartyToTriggerEvent,
            TDelegate<void(bool)>::CreateWeakLambda(this, [this, LocalUserNum](bool bSucceeded)
            {
                if (bSucceeded)
                {
                    UE_LOG_ONLINESESSION(Log, TEXT("Creating an initial party."));
                    CreateParty(LocalUserNum);
                }
                else 
                {
                    UE_LOG_ONLINESESSION(Warning, TEXT("Failed to create an initial party. Restoring party session was failed."));
                }
            })
    ));
}
#pragma endregion