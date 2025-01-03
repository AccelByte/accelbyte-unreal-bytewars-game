// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "PlayingWithFriendsSubsystem_Starter.h"

#include "PlayingWithFriendsLog.h"
#include "PlayingWithFriendsModels.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineUserInterface.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UPlayingWithFriendsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get online session
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(OnlineSession))
	{
		return;
	}

	// TODO: Add your Online Session delegates setup here
}

void UPlayingWithFriendsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Add your Online Session delegates cleanup here
}

#pragma region "Helper"
bool UPlayingWithFriendsSubsystem_Starter::IsInMatchSessionGameSession() const
{
	FString RequestedSessionType;
	const FNamedOnlineSession* Session = OnlineSession->GetSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	if (!Session)
	{
		return false;
	}

	Session->SessionSettings.Get(GAME_SESSION_REQUEST_TYPE, RequestedSessionType);
	if (RequestedSessionType == GAME_SESSION_REQUEST_TYPE_MATCHSESSION)
	{
		return true;
	}

	return false;
}

bool UPlayingWithFriendsSubsystem_Starter::IsMatchSessionGameSessionReceivedServer() const
{
	if (!GetSessionInterface() || !GetIdentityInterface())
	{
		return false;
	}

	FUniqueNetIdPtr UserId = nullptr;
	if (GetIdentityInterface())
	{
		UserId = GetIdentityInterface()->GetUniquePlayerId(0);
	}

	if (!UserId)
	{
		return false;
	}

	const FName SessionName = OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession);

	FString ServerAddress = TEXT("");
	GetSessionInterface()->GetResolvedConnectString(SessionName, ServerAddress);
	const bool bIsP2PHost = GetSessionInterface()->IsPlayerP2PHost(UserId.ToSharedRef().Get(), SessionName);

	return !ServerAddress.IsEmpty() || bIsP2PHost;
}

FUniqueNetIdRef UPlayingWithFriendsSubsystem_Starter::GetSessionOwnerUniqueNetId(const FName SessionName) const
{
	const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName);
	return Session->LocalOwnerId.Get()->AsShared();
}

UPromptSubsystem* UPlayingWithFriendsSubsystem_Starter::GetPromptSubsystem() const
{
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}

FOnlineSessionV2AccelBytePtr UPlayingWithFriendsSubsystem_Starter::GetSessionInterface() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(World));
}

FOnlineIdentityAccelBytePtr UPlayingWithFriendsSubsystem_Starter::GetIdentityInterface() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Online::GetIdentityInterface(World));
}

void UPlayingWithFriendsSubsystem_Starter::JoinGameSessionConfirmation(
	const int32 LocalUserNum,
	const FOnlineSessionInviteAccelByte& Invite)
{
	if (OnlineSession->GetSession(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		UPromptSubsystem* PromptSubsystem = GetPromptSubsystem();
		if (!PromptSubsystem)
		{
			return;
		}

		PromptSubsystem->ShowDialoguePopUp(
			TEXT_LEAVING_SESSION,
			TEXT_JOIN_NEW_SESSION_CONFIRMATION,
			EPopUpType::ConfirmationConfirmCancel,
			FPopUpResultDelegate::CreateWeakLambda(this, [this, LocalUserNum, Invite](EPopUpResult Result)
			{
				switch (Result)
				{
				case Confirmed:
					// TODO: call join session
					break;
				case Declined:
					// TODO: call reject invite
					break;
				}
			}));
	}
	else
	{
		UPromptSubsystem* PromptSubsystem = GetPromptSubsystem();
		if (!PromptSubsystem)
		{
			return;
		}

		PromptSubsystem->ShowLoading(TEXT_JOINING_SESSION);
		// TODO: call join session
	}
}

void UPlayingWithFriendsSubsystem_Starter::OnQueryUserInfoOnGameSessionParticipantChange(
	const FOnlineError& Error,
	const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
	FName SessionName,
	const bool bJoined)
{
	// Abort if not a game session.
	if (!OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
	{
		return;
	}

	// Only handle the event if the player already travelled to online session.
	if (GetWorld() && GetWorld()->GetNetMode() == ENetMode::NM_Standalone)
	{
		return;
	}

	// Only handle the event if the game session is in the game server.
	if (!IsMatchSessionGameSessionReceivedServer())
	{
		return;
	}

	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Called"));

	UPromptSubsystem* PromptSubsystem = GetPromptSubsystem();
	if (!Error.bSucceeded || UsersInfo.IsEmpty() || !PromptSubsystem)
	{
		return;
	}

	for (const TSharedPtr<FUserOnlineAccountAccelByte>& User : UsersInfo)
	{
		const bool bIsLeaderInfo = OnlineSession->CompareAccelByteUniqueId(
			FUniqueNetIdRepl(User->GetUserId()),
			FUniqueNetIdRepl(LeaderId));
		// Joined / left member
		if (!bIsLeaderInfo)
		{
			UE_LOG_PLAYINGWITHFRIENDS(VeryVerbose, TEXT("Member %s"), *FString(bJoined ? "JOINED" : "LEFT"));

			const FText Message = FText::Format(
				bJoined ? TEXT_FORMAT_JOINED : TEXT_FORMAT_LEFT,
				FText::FromString(User->GetDisplayName().IsEmpty()
					? UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(User->GetUserId().Get())
					: User->GetDisplayName()));

			FString AvatarURL;
			User->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

			PromptSubsystem->PushNotification(Message, AvatarURL);
		}

		// Session's leader change
		else if (bIsLeaderInfo && bLeaderChanged)
		{
			bLeaderChanged = false;

			const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName);
			if (!Session)
			{
				break;
			}

			const bool bIsLocalUserLeader = OnlineSession->CompareAccelByteUniqueId(
				FUniqueNetIdRepl(Session->LocalOwnerId),
				FUniqueNetIdRepl(LeaderId));
			const FText Message = FText::Format(
				TEXT_FORMAT_LEADER_CHANGED,
				bIsLocalUserLeader ? TEXT_YOU_ARE : FText::FromString(User->GetDisplayName().IsEmpty() ?
					UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(User->GetUserId().Get())
					: User->GetDisplayName()));

			FString AvatarURL;
			User->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

			PromptSubsystem->PushNotification(Message, AvatarURL);
		}
	}
}
#pragma endregion 

#pragma region "Playing with Friends implementations"
// TODO: Add your module implementations here.
#pragma endregion 
