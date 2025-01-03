// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "SessionEssentialsOnlineSession.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "SessionEssentialsLog.h"

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-RegisterOnlineDelegates
// @@@MULTISNIP BindCreateSessionDelegate {"selectedLines": ["1-2", "5-6", "27"]}
// @@@MULTISNIP BindJoinSessionDelegate {"selectedLines": ["1-2", "7-8", "27"]}
// @@@MULTISNIP BindSessionInvitationDelegate {"selectedLines": ["1-2", "9-10", "16-17", "27"]}
// @@@MULTISNIP BindLeaveSessionDelegate {"selectedLines": ["1-2", "11-12", "27"]}
// @@@MULTISNIP BindUpdateSessionDelegate {"selectedLines": ["1-2", "13-14", "27"]}
// @@@MULTISNIP BindSessionParticipantDelegate {"selectedLines": ["1-2", "18-27"]}
void USessionEssentialsOnlineSession::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	GetSessionInt()->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	GetSessionInt()->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	GetABSessionInt()->AddOnSendSessionInviteCompleteDelegate_Handle(
		FOnSendSessionInviteCompleteDelegate::CreateUObject(this, &ThisClass::OnSendSessionInviteComplete));
	GetSessionInt()->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnLeaveSessionComplete));
	GetSessionInt()->AddOnUpdateSessionCompleteDelegate_Handle(
		FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionComplete));

	GetABSessionInt()->AddOnV2SessionInviteReceivedDelegate_Handle(
		FOnV2SessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived));
#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	GetABSessionInt()->AddOnSessionParticipantsChangeDelegate_Handle(
		FOnSessionParticipantsChangeDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsChange));
#else
	GetABSessionInt()->AddOnSessionParticipantJoinedDelegate_Handle(
		FOnSessionParticipantJoinedDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantJoined));
	GetABSessionInt()->AddOnSessionParticipantLeftDelegate_Handle(
		FOnSessionParticipantLeftDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantLeft));
#endif
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-ClearOnlineDelegates
// @@@MULTISNIP UnbindCreateSessionDelegate {"selectedLines": ["1-2", "5", "19"]}
// @@@MULTISNIP UnbindJoinSessionDelegate {"selectedLines": ["1-2", "6", "19"]}
// @@@MULTISNIP UnbindSessionInvitationDelegate {"selectedLines": ["1-2", "7", "11", "19"]}
// @@@MULTISNIP UnbindLeaveSessionDelegate {"selectedLines": ["1-2", "8", "19"]}
// @@@MULTISNIP UnbindUpdateSessionDelegate {"selectedLines": ["1-2", "9", "19"]}
// @@@MULTISNIP UnbindSessionParticipantDelegate {"selectedLines": ["1-2", "13-19"]}
void USessionEssentialsOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	GetSessionInt()->ClearOnCreateSessionCompleteDelegates(this);
	GetSessionInt()->ClearOnJoinSessionCompleteDelegates(this);
	GetABSessionInt()->ClearOnSendSessionInviteCompleteDelegates(this);
	GetSessionInt()->ClearOnDestroySessionCompleteDelegates(this);
	GetSessionInt()->ClearOnUpdateSessionCompleteDelegates(this);

	GetABSessionInt()->ClearOnV2SessionInviteReceivedDelegates(this);

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	GetABSessionInt()->ClearOnSessionParticipantsChangeDelegates(this);
#else
	GetABSessionInt()->ClearOnSessionParticipantJoinedDelegates(this);
	GetABSessionInt()->ClearOnSessionParticipantLeftDelegates(this);
#endif
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-GetSession
FNamedOnlineSession* USessionEssentialsOnlineSession::GetSession(const FName SessionName)
{
	return GetSessionInt()->GetNamedSession(SessionName);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-GetSessionType
EAccelByteV2SessionType USessionEssentialsOnlineSession::GetSessionType(const FName SessionName)
{
	const FNamedOnlineSession* OnlineSession = GetSession(SessionName);
	if (!OnlineSession)
	{
		return EAccelByteV2SessionType::Unknown;
	}

	const FOnlineSessionSettings& OnlineSessionSettings = OnlineSession->SessionSettings;

	return GetABSessionInt()->GetSessionTypeFromSettings(OnlineSessionSettings);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-GetPredefinedSessionNameFromType
FName USessionEssentialsOnlineSession::GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType)
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
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-CreateSession
void USessionEssentialsOnlineSession::CreateSession(
	const int32 LocalUserNum,
	FName SessionName,
	FOnlineSessionSettings SessionSettings,
	const EAccelByteV2SessionType SessionType,
	const FString& SessionTemplateName)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!GetSessionInt())
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Session interface is null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnCreateSessionComplete(SessionName, false);
		}));
		return;
	}
	if (SessionTemplateName.IsEmpty())
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Session Template Name can't be empty"))
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
		FString ServerName;
		FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);
		if (!ServerName.IsEmpty())
		{
			UE_LOG_SESSIONESSENTIALS(Log, TEXT("Requesting to use server with name: %s"), *ServerName)
			SessionSettings.Set(SETTING_GAMESESSION_SERVERNAME, ServerName);
		}
	}
#pragma endregion

	// If the session exists locally, then destroy the session first.
	if (GetSession(SessionName))
	{
		UE_LOG_SESSIONESSENTIALS(Log, TEXT("The session exists locally. Leaving session first."))

		// Reset the delegate.
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
			UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnCreateSessionComplete(SessionName, false);
			}));
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-JoinSession
void USessionEssentialsOnlineSession::JoinSession(
	const int32 LocalUserNum,
	FName SessionName,
	const FOnlineSessionSearchResult& SearchResult)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!GetSessionInt())
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Session interface is null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		}));
		return;
	}

	// If the session exist, then destroy it first and then join the new one.
	if (GetSession(SessionName))
	{
		// Reset the delegate.
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
			UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}));
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-SendSessionInvite
void USessionEssentialsOnlineSession::SendSessionInvite(
	const int32 LocalUserNum,
	FName SessionName,
	const FUniqueNetIdPtr Invitee)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("Called"));

	if (!Invitee.IsValid())
	{
		UE_LOG_SESSIONESSENTIALS(Log, TEXT("Invitee net id is invalid. Canceling operation"));
		return;
	}

	GetABSessionInt()->SendSessionInviteToFriend(LocalUserNum, SessionName, *Invitee.Get());
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-RejectSessionInvite
void USessionEssentialsOnlineSession::RejectSessionInvite(
	const int32 LocalUserNum,
	const FOnlineSessionInviteAccelByte& Invite)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("Called"));

	const FUniqueNetIdPtr LocalUserNetId = GetLocalPlayerUniqueNetId(GetPlayerControllerByLocalUserNum(LocalUserNum));
	if (!LocalUserNetId.IsValid())
	{
		UE_LOG_SESSIONESSENTIALS(Log, TEXT("Local User net id is invalid. Canceling operation"));
		return;
	}

	GetABSessionInt()->RejectInvite(
		*LocalUserNetId.Get(),
		Invite,
		FOnRejectSessionInviteComplete::CreateUObject(this, &ThisClass::OnRejectSessionInviteComplete));
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-LeaveSession
void USessionEssentialsOnlineSession::LeaveSession(FName SessionName)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("called"))

	// Abort if the session interface is invalid.
	if (!GetSessionInt())
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Session interface is null"))
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
			UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Failed to execute"))
			ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
			{
				OnLeaveSessionComplete(SessionName, false);
			}));
		}
		else
		{
			bLeavingSession = true;
		}
	}
	else
	{
		UE_LOG_SESSIONESSENTIALS(Log, TEXT("Not in session"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
		{
			OnLeaveSessionComplete(SessionName, true);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-UpdateSessionJoinability
void USessionEssentialsOnlineSession::UpdateSessionJoinability(const FName SessionName, const EAccelByteV2SessionJoinability Joinability)
{
	UE_LOG_SESSIONESSENTIALS(Verbose, TEXT("called"));

	FOnlineSessionV2AccelBytePtr ABSessionInt = GetABSessionInt();
	if (!ABSessionInt)
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Session interface is null"));
		return;
	}

	FNamedOnlineSession* Session = ABSessionInt->GetNamedSession(SessionName);
	if (!Session)
	{
		UE_LOG_SESSIONESSENTIALS(Warning, TEXT("The session is invalid"));
		return;
	}

	Session->SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, UEnum::GetValueAsString(Joinability));
	ABSessionInt->UpdateSession(SessionName, Session->SessionSettings);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnCreateSessionComplete
void USessionEssentialsOnlineSession::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnCreateSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnJoinSessionComplete
void USessionEssentialsOnlineSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(Result == EOnJoinSessionCompleteResult::Success ? "TRUE" : "FALSE"))

	OnJoinSessionCompleteDelegates.Broadcast(SessionName, Result);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnSendSessionInviteComplete
void USessionEssentialsOnlineSession::OnSendSessionInviteComplete(
	const FUniqueNetId& LocalSenderId,
	FName SessionName,
	bool bSucceeded,
	const FUniqueNetId& InviteeId)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	OnSendSessionInviteCompleteDelegates.Broadcast(LocalSenderId, SessionName, bSucceeded, InviteeId);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnRejectSessionInviteComplete
void USessionEssentialsOnlineSession::OnRejectSessionInviteComplete(bool bSucceeded)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	OnRejectSessionInviteCompleteDelegates.Broadcast(bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnLeaveSessionComplete
void USessionEssentialsOnlineSession::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bLeavingSession = false;
	OnLeaveSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnUpdateSessionComplete
void USessionEssentialsOnlineSession::OnUpdateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	OnUpdateSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnSessionInviteReceived
void USessionEssentialsOnlineSession::OnSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("from: %s"), *FromId.ToDebugString())

	OnSessionInviteReceivedDelegates.Broadcast(UserId, FromId, Invite);
}
// @@@SNIPEND

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnSessionParticipantsChange
void USessionEssentialsOnlineSession::OnSessionParticipantsChange(
	FName SessionName,
	const FUniqueNetId& Member,
	bool bJoined)
{
	UE_LOG_SESSIONESSENTIALS(
		Log,
		TEXT("The session name: %s | Member: %s [%s]"),
		*SessionName.ToString(),
		*Member.ToDebugString(),
		*FString(bJoined ? "Joined" : "Left"))

		OnSessionParticipantsChangeDelegates.Broadcast(SessionName, Member, bJoined);
}
// @@@SNIPEND
#else
// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnSessionParticipantJoined
void USessionEssentialsOnlineSession::OnSessionParticipantJoined(
	FName SessionName,
	const FUniqueNetId& Member)
{
	UE_LOG_SESSIONESSENTIALS(
		Log,
		TEXT("Session name: %s | Member: %s [Joined]"),
		*SessionName.ToString(),
		*Member.ToDebugString())

	OnSessionParticipantJoinedDelegates.Broadcast(SessionName, Member);
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnSessionParticipantLeft
void USessionEssentialsOnlineSession::OnSessionParticipantLeft(
	FName SessionName,
	const FUniqueNetId& Member,
	EOnSessionParticipantLeftReason Reason)
{
	UE_LOG_SESSIONESSENTIALS(
		Log,
		TEXT("Session name: %s | Member: %s [Left] | Reason: %s"),
		*SessionName.ToString(),
		*Member.ToDebugString(),
		ToLogString(Reason))

		OnSessionParticipantLeftDelegates.Broadcast(SessionName, Member, Reason);
}
// @@@SNIPEND
#endif

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnLeaveSessionForCreateSessionComplete
void USessionEssentialsOnlineSession::OnLeaveSessionForCreateSessionComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const FOnlineSessionSettings SessionSettings)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForCreateSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->CreateSession(LocalUserNum, SessionName, SessionSettings))
		{
			UE_LOG_SESSIONESSENTIALS(Warning, TEXT("Failed to execute"))
			OnCreateSessionComplete(SessionName, false);
		}
	}
	else
	{
		// Leave session failed, execute complete delegate as failed.
		OnCreateSessionComplete(SessionName, false);
	}
}
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.cpp-OnLeaveSessionForJoinSessionComplete
void USessionEssentialsOnlineSession::OnLeaveSessionForJoinSessionComplete(
	FName SessionName,
	bool bSucceeded,
	const int32 LocalUserNum,
	const FOnlineSessionSearchResult SearchResult)
{
	UE_LOG_SESSIONESSENTIALS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))
	OnLeaveSessionCompleteDelegates.Remove(OnLeaveSessionForJoinSessionCompleteDelegateHandle);

	if (bSucceeded)
	{
		if (!GetSessionInt()->JoinSession(LocalUserNum, SessionName, SearchResult))
		{
			UE_LOG_SESSIONESSENTIALS(Warning, TEXT("failed to execute"))
			OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		}
	}
	else
	{
		// Leave session failed, execute complete delegate as failed.
		OnJoinSessionComplete(SessionName, EOnJoinSessionCompleteResult::UnknownError);
	}
}
// @@@SNIPEND
