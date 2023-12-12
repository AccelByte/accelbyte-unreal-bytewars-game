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

#pragma region "Helper"
public:
	bool IsInMatchSessionGameSession() const;

private:
	FUniqueNetIdRef GetSessionOwnerUniqueNetId(const FName SessionName) const;
	UPromptSubsystem* GetPromptSubsystem() const;

	void JoinGameSessionConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& Invite);
	void OnQueryUserInfoOnGameSessionParticipantChange(
		const bool bSucceeded,
		const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
		FName SessionName,
		const bool bJoined);
#pragma endregion 

public:
	void SendGameSessionInvite(const APlayerController* Owner, const FUniqueNetIdPtr Invitee) const;
	FOnSendSessionInviteComplete OnSendGameSessionInviteCompleteDelegates;

	void RejectGameSessionInvite(const APlayerController* Owner, const FOnlineSessionInviteAccelByte& Invite) const;
	FOnRejectSessionInviteCompleteMulticast OnRejectGameSessionInviteCompleteDelegates;

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
		const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
		const int32 LocalUserNum, const FOnlineSessionInviteAccelByte Invite);

	void OnGameSessionParticipantsChange(FName SessionName, const FUniqueNetId& Member, bool bJoined);

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

	FUniqueNetIdPtr LeaderId;
	bool bLeaderChanged = false;
};
