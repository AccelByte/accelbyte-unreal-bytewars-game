// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "PlayingWithFriendsSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPlayingWithFriendsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Don't run on DS
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return IsRunningDedicatedServer() ? false : Super::ShouldCreateSubsystem(Outer);
	}

// @@@SNIPSTART PlayingWithFriendsSubsystem.h-public
// @@@MULTISNIP SendGameSessionInvite {"selectedLines": ["1-2"]}
// @@@MULTISNIP SendGameSessionInviteDelegate {"selectedLines": ["1", "3"]}
// @@@MULTISNIP RejectGameSessionInvite {"selectedLines": ["1", "5"]}
// @@@MULTISNIP RejectGameSessionInviteDelegate {"selectedLines": ["1", "6"]}
// @@@MULTISNIP HelperFunction {"selectedLines": ["1", "9"]}
public:
	void SendGameSessionInvite(const APlayerController* Owner, const FUniqueNetIdPtr Invitee) const;
	FOnSendSessionInviteComplete OnSendGameSessionInviteCompleteDelegates;

	void RejectGameSessionInvite(const APlayerController* Owner, const FOnlineSessionInviteAccelByte& Invite) const;
	FOnRejectSessionInviteCompleteMulticast OnRejectGameSessionInviteCompleteDelegates;

#pragma region "Helper"
	bool IsInMatchSessionGameSession() const;
#pragma endregion 
// @@@SNIPEND

// @@@SNIPSTART PlayingWithFriendsSubsystem.h-private
// @@@MULTISNIP OnSendGameSessionInviteComplete {"selectedLines": ["1-6"]}
// @@@MULTISNIP OnRejectGameSessionInviteComplete {"selectedLines": ["1", "7"]}
// @@@MULTISNIP JoinGameSession {"selectedLines": ["1", "9"]}
// @@@MULTISNIP OnJoinSessionComplete {"selectedLines": ["1", "10"]}
// @@@MULTISNIP OnGameSessionInviteReceived {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP ShowInviteReceivedPopup {"selectedLines": ["1", "16-18"]}
// @@@MULTISNIP OnGameSessionParticipantsChange {"selectedLines": ["1", "20-25"]}
// @@@MULTISNIP HelperFunction {"selectedLines": ["1", "28-41"]}
// @@@MULTISNIP HelperVariable {"selectedLines": ["1", "44-48"]}
private:
	void OnSendGameSessionInviteComplete(
		const FUniqueNetId& LocalSenderId,
		FName SessionName,
		bool bSucceeded,
		const FUniqueNetId& InviteeId) const;
	void OnRejectGameSessionInviteComplete(bool bSucceeded) const;

	void JoinGameSession(const int32 LocalUserNum, const FOnlineSessionSearchResult& Session) const;
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type CompletionType) const;

	void OnGameSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FOnlineSessionInviteAccelByte& Invite);
	void ShowInviteReceivedPopup(
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
		const int32 LocalUserNum, const FOnlineSessionInviteAccelByte Invite);

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	void OnGameSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member, bool bJoined);
#else
	void OnGameSessionParticipantJoined(FName SessionName, const FUniqueNetId& Member);
	void OnGameSessionParticipantLeft(FName SessionName, const FUniqueNetId& Member, EOnSessionParticipantLeftReason Reason);
#endif

#pragma region "Helper"
	bool IsMatchSessionGameSessionReceivedServer() const;
	
	FUniqueNetIdRef GetSessionOwnerUniqueNetId(const FName SessionName) const;
	UPromptSubsystem* GetPromptSubsystem() const;

	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	void JoinGameSessionConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& Invite);
	void OnQueryUserInfoOnGameSessionParticipantChange(
		const FOnlineError& Error,
		const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
		FName SessionName,
		const bool bJoined);
#pragma endregion 

	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	FUniqueNetIdPtr LeaderId;
	bool bLeaderChanged = false;
// @@@SNIPEND
};
