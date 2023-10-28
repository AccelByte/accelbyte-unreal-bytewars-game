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

	virtual FOnV2SessionInviteReceived* GetOnSessionInviteReceivedDelegates() override
	{
		return &OnSessionInviteReceivedDelegates;
	}
	virtual FOnSessionParticipantsChange* GetOnSessionParticipantsChange() override
	{
		return &OnSessionParticipantsChangeDelegates;
	}

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

	virtual void OnSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FOnlineSessionInviteAccelByte& Invite) override;
	virtual void OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member, bool bJoined) override;

private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnSendSessionInviteComplete OnSendSessionInviteCompleteDelegates;
	FOnRejectSessionInviteCompleteMulticast OnRejectSessionInviteCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;

	FOnV2SessionInviteReceived OnSessionInviteReceivedDelegates;
	FOnSessionParticipantsChange OnSessionParticipantsChangeDelegates;

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
};
