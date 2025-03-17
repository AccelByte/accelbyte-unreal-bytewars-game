// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSession.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "SessionEssentialsOnlineSession.generated.h"

UCLASS()
class ACCELBYTEWARS_API USessionEssentialsOnlineSession : public UAccelByteWarsOnlineSessionBase
{
	GENERATED_BODY()

// @@@SNIPSTART SessionEssentialsOnlineSession.h-public
// @@@MULTISNIP CreateSession {"selectedLines": ["1", "9-14"]}
// @@@MULTISNIP JoinSession {"selectedLines": ["1", "15-18"]}
// @@@MULTISNIP SendSessionInvite {"selectedLines": ["1", "19"]}
// @@@MULTISNIP RejectSessionInvite {"selectedLines": ["1", "20"]}
// @@@MULTISNIP LeaveSession {"selectedLines": ["1", "21"]}
// @@@MULTISNIP UpdateSessionJoinability {"selectedLines": ["1", "22"]}
// @@@MULTISNIP SessionDelegates {"selectedLines": ["1", "24-53", "60-67"]}
public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

	virtual FNamedOnlineSession* GetSession(const FName SessionName) override;
	virtual EAccelByteV2SessionType GetSessionType(const FName SessionName) override;
	virtual FName GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType) override;

	virtual void CreateSession(
		const int32 LocalUserNum,
		FName SessionName,
		FOnlineSessionSettings SessionSettings,
		const EAccelByteV2SessionType SessionType,
		const FString& SessionTemplateName) override;
	virtual void JoinSession(
		const int32 LocalUserNum,
		FName SessionName,
		const FOnlineSessionSearchResult& SearchResult) override;
	virtual void SendSessionInvite(const int32 LocalUserNum, FName SessionName, const FUniqueNetIdPtr Invitee) override;
	virtual void RejectSessionInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& Invite) override;
	virtual void LeaveSession(FName SessionName) override;
	virtual void UpdateSessionJoinability(const FName SessionName, const EAccelByteV2SessionJoinability Joinability) override;

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates() override
	{
		return &OnCreateSessionCompleteDelegates;
	}
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates() override
	{
		return &OnJoinSessionCompleteDelegates;
	}
	virtual FOnSendSessionInviteComplete* GetOnSendSessionInviteCompleteDelegates() override
	{
		return &OnSendSessionInviteCompleteDelegates;
	}
	virtual FOnRejectSessionInviteCompleteMulticast* GetOnRejectSessionInviteCompleteDelegate() override
	{
		return &OnRejectSessionInviteCompleteDelegates;
	}
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates() override
	{
		return &OnLeaveSessionCompleteDelegates;
	}
	virtual FOnUpdateSessionComplete* GetOnUpdateSessionCompleteDelegates() override
	{
		return &OnUpdateSessionCompleteDelegates;
	}

	virtual FOnV2SessionInviteReceived* GetOnSessionInviteReceivedDelegates() override
	{
		return &OnSessionInviteReceivedDelegates;
	}

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	virtual FOnSessionParticipantsChange* GetOnSessionParticipantsChange() override
	{
		return &OnSessionParticipantsChangeDelegates;
	}
#else
	virtual FOnSessionParticipantJoined* GetOnSessionParticipantJoined() override
	{
		return &OnSessionParticipantJoinedDelegates;
	}
	virtual FOnSessionParticipantLeft* GetOnSessionParticipantLeft() override
	{
		return &OnSessionParticipantLeftDelegates;
	}
#endif
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.h-protected
// @@@MULTISNIP LeaveSessionHelperVariable {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnCreateSessionComplete {"selectedLines": ["1", "4"]}
// @@@MULTISNIP OnJoinSessionComplete {"selectedLines": ["1", "6"]}
// @@@MULTISNIP OnSendSessionInviteComplete {"selectedLines": ["1", "7-11"]}
// @@@MULTISNIP OnRejectSessionInviteComplete {"selectedLines": ["1", "12"]}
// @@@MULTISNIP OnLeaveSessionComplete {"selectedLines": ["1", "13"]}
// @@@MULTISNIP OnUpdateSessionComplete {"selectedLines": ["1", "14"]}
// @@@MULTISNIP OnSessionInviteReceived {"selectedLines": ["1", "16-19"]}
// @@@MULTISNIP OnSessionParticipantsChange {"selectedLines": ["1", "24-25"]}
protected:
	bool bLeavingSession = false;

	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded) override;
	/*The parent's function with the same name will not be used. Ignore the "hides a non-virtual function" warning*/
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;
	virtual void OnSendSessionInviteComplete(
		const FUniqueNetId& LocalSenderId,
		FName SessionName,
		bool bSucceeded,
		const FUniqueNetId& InviteeId) override;
	virtual void OnRejectSessionInviteComplete(bool bSucceeded) override;
	virtual void OnLeaveSessionComplete(FName SessionName, bool bSucceeded) override;
	virtual void OnUpdateSessionComplete(FName SessionName, bool bSucceeded) override;

	virtual void OnSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FOnlineSessionInviteAccelByte& Invite) override;

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	virtual void OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member, bool bJoined) override;
#else
	virtual void OnSessionParticipantJoined(FName SessionName, const FUniqueNetId& Member) override;
	virtual void OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& Member, EOnSessionParticipantLeftReason Reason) override;
#endif
// @@@SNIPEND

// @@@SNIPSTART SessionEssentialsOnlineSession.h-private
// @@@MULTISNIP OnLeaveSessionForCreateSessionComplete {"selectedLines": ["1", "21-26"]}
// @@@MULTISNIP OnLeaveSessionForJoinSessionComplete {"selectedLines": ["1", "28-33"]}
// @@@MULTISNIP SessionDelegates {"selectedLines": ["1", "5-13", "17-18"]}
private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnSendSessionInviteComplete OnSendSessionInviteCompleteDelegates;
	FOnRejectSessionInviteCompleteMulticast OnRejectSessionInviteCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;
	FOnUpdateSessionComplete OnUpdateSessionCompleteDelegates;

	FOnV2SessionInviteReceived OnSessionInviteReceivedDelegates;

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	FOnSessionParticipantsChange OnSessionParticipantsChangeDelegates;
#else
	FOnSessionParticipantJoined OnSessionParticipantJoinedDelegates;
	FOnSessionParticipantLeft OnSessionParticipantLeftDelegates;
#endif

	void OnLeaveSessionForCreateSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSettings SessionSettings);
	FDelegateHandle OnLeaveSessionForCreateSessionCompleteDelegateHandle;

	void OnLeaveSessionForJoinSessionComplete(
		FName SessionName,
		bool bSucceeded,
		const int32 LocalUserNum,
		const FOnlineSessionSearchResult SearchResult);
	FDelegateHandle OnLeaveSessionForJoinSessionCompleteDelegateHandle;
// @@@SNIPEND
};
