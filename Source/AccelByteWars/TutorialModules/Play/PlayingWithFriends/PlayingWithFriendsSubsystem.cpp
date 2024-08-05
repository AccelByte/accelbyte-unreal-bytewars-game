// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "PlayingWithFriendsSubsystem.h"

#include "PlayingWithFriendsLog.h"
#include "PlayingWithFriendsModels.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineUserInterface.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UPlayingWithFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Online Session
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(OnlineSession))
	{
		return;
	}

	OnlineSession->GetOnSendSessionInviteCompleteDelegates()->AddUObject(
		this, &ThisClass::OnSendGameSessionInviteComplete);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->AddUObject(
		this, &ThisClass::OnJoinSessionComplete);
	OnlineSession->GetOnRejectSessionInviteCompleteDelegate()->AddUObject(
		this, &ThisClass::OnRejectGameSessionInviteComplete);
	OnlineSession->GetOnSessionInviteReceivedDelegates()->AddUObject(
		this, &ThisClass::OnGameSessionInviteReceived);
	OnlineSession->GetOnSessionParticipantsChange()->AddUObject(
		this, &ThisClass::OnGameSessionParticipantsChange);
}

void UPlayingWithFriendsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	OnlineSession->GetOnSendSessionInviteCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnRejectSessionInviteCompleteDelegate()->RemoveAll(this);
	OnlineSession->GetOnSessionInviteReceivedDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionParticipantsChange()->RemoveAll(this);
}

#pragma region "Helper"
bool UPlayingWithFriendsSubsystem::IsInMatchSessionGameSession() const
{
	FString RequestedSessionType;
	const FNamedOnlineSession* Session = OnlineSession->GetSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	if (!Session)
	{
		return false;
	}

	// Only handle the event if the game session is in the game server.
	if (!IsMatchSessionGameSessionReceivedServer())
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

bool UPlayingWithFriendsSubsystem::IsMatchSessionGameSessionReceivedServer() const
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

FUniqueNetIdRef UPlayingWithFriendsSubsystem::GetSessionOwnerUniqueNetId(const FName SessionName) const
{
	const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName);
	return Session->LocalOwnerId.Get()->AsShared();
}

UPromptSubsystem* UPlayingWithFriendsSubsystem::GetPromptSubsystem() const
{
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}

FOnlineSessionV2AccelBytePtr UPlayingWithFriendsSubsystem::GetSessionInterface() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(World));
}

FOnlineIdentityAccelBytePtr UPlayingWithFriendsSubsystem::GetIdentityInterface() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Online::GetIdentityInterface(World));
}

void UPlayingWithFriendsSubsystem::JoinGameSessionConfirmation(
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
					JoinGameSession(LocalUserNum, Invite.Session);
					break;
				case Declined:
					OnlineSession->RejectSessionInvite(LocalUserNum, Invite);
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
		JoinGameSession(LocalUserNum, Invite.Session);
	}
}

void UPlayingWithFriendsSubsystem::OnQueryUserInfoOnGameSessionParticipantChange(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
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
	if (!bSucceeded || UsersInfo.IsEmpty() || !PromptSubsystem)
	{
		return;
	}

	for (const FUserOnlineAccountAccelByte* User : UsersInfo)
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

void UPlayingWithFriendsSubsystem::SendGameSessionInvite(const APlayerController* Owner, const FUniqueNetIdPtr Invitee) const
{
	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Called"));

	OnlineSession->SendSessionInvite(
		OnlineSession->GetLocalUserNumFromPlayerController(Owner),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Invitee);
}

void UPlayingWithFriendsSubsystem::RejectGameSessionInvite(
	const APlayerController* Owner,
	const FOnlineSessionInviteAccelByte& Invite) const
{
	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Called"));

	OnlineSession->RejectSessionInvite(
		OnlineSession->GetLocalUserNumFromPlayerController(Owner),
		Invite);
}

void UPlayingWithFriendsSubsystem::OnSendGameSessionInviteComplete(
	const FUniqueNetId& LocalSenderId,
	FName SessionName,
	bool bSucceeded,
	const FUniqueNetId& InviteeId) const
{
	// Abort if not a game session.
	if (!OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
	{
		return;
	}

	// Only handle the event if the game session is in the game server.
	if (!IsMatchSessionGameSessionReceivedServer())
	{
		return;
	}

	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Succeedded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));

	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	// show sent push notification
	if (UPromptSubsystem* PromptSubsystem = GetPromptSubsystem())
	{
		PromptSubsystem->PushNotification(TEXT_INVITE_SENT);
	}

	OnSendGameSessionInviteCompleteDelegates.Broadcast(LocalSenderId, SessionName, bSucceeded, InviteeId);
}

void UPlayingWithFriendsSubsystem::OnRejectGameSessionInviteComplete(bool bSucceeded) const
{
	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Succeedded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));

	OnRejectGameSessionInviteCompleteDelegates.Broadcast(bSucceeded);
}

void UPlayingWithFriendsSubsystem::JoinGameSession(const int32 LocalUserNum, const FOnlineSessionSearchResult& Session) const
{
	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Called"));

	OnlineSession->JoinSession(
		LocalUserNum,
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Session);
}

void UPlayingWithFriendsSubsystem::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type CompletionType) const
{
	// Abort if not a game session.
	if (!OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
	{
		return;
	}

	// Only handle the event if the game session is in the game server.
	if (!IsMatchSessionGameSessionReceivedServer())
	{
		return;
	}

	const bool bSucceeded = CompletionType == EOnJoinSessionCompleteResult::Success;
	FText ErrorMessage;

	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Succeedded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));

	switch (CompletionType)
	{
	case EOnJoinSessionCompleteResult::Success:
		ErrorMessage = FText();
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ErrorMessage = TEXT_FAILED_SESSION_FULL_PLAYING_WITH_FRIENDS;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ErrorMessage = TEXT_FAILED_SESSION_NULL_PLAYING_WITH_FRIENDS;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ErrorMessage = TEXT_FAILED_TO_JOIN_SESSION_PLAYING_WITH_FRIENDS;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ErrorMessage = TEXT_FAILED_ALREADY_IN_SESSION_PLAYING_WITH_FRIENDS;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		ErrorMessage = TEXT_FAILED_TO_JOIN_SESSION_PLAYING_WITH_FRIENDS;
		break;
	default:
		ErrorMessage = FText();
	}

	if (UPromptSubsystem* PromptSubsystem = GetPromptSubsystem(); !bSucceeded && PromptSubsystem)
	{
		PromptSubsystem->PushNotification(ErrorMessage);
	}
}

void UPlayingWithFriendsSubsystem::OnGameSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	/* Make sure it is a game session.
	 * Also check if the inviter is not from party leader.
	 * Since the party members will automatically join, thus no need to show game session invitation notif.*/
	if (UserId == FromId || 
		Invite.SessionType != EAccelByteV2SessionType::GameSession ||
		OnlineSession->IsPartyLeader(FromId.AsShared()))
	{
		return;
	}

	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Invite received from: %s"), *FromId.ToDebugString());

	const APlayerController* PlayerController = OnlineSession->GetPlayerControllerByUniqueNetId(UserId.AsShared());
	if (!PlayerController)
	{
		return;
	}

	const int32 LocalUserNum = OnlineSession->GetLocalUserNumFromPlayerController(PlayerController);
	if (LocalUserNum == INDEX_NONE)
	{
		return;
	}

	const FUniqueNetIdAccelByteUserRef FromABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(FromId.AsShared());
	OnlineSession->QueryUserInfo(
		LocalUserNum,
		TPartyMemberArray{FromABId},
		FOnQueryUsersInfoComplete::CreateWeakLambda(
			this,
			[this, Invite, LocalUserNum](const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
			{
				/**
				 * For some reason, calling ShowInviteReceivedPopup through CreateUObject crashes the game.
				 * WeakLambda used in place of that.
				 */
				if (bSucceeded)
				{
					ShowInviteReceivedPopup(UsersInfo, LocalUserNum, Invite);
				}
			}));
}

void UPlayingWithFriendsSubsystem::ShowInviteReceivedPopup(
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
	const int32 LocalUserNum,
	const FOnlineSessionInviteAccelByte Invite)
{
	UPromptSubsystem* PromptSubsystem = GetPromptSubsystem();
	if (UsersInfo.IsEmpty() || !PromptSubsystem)
	{
		return;
	}

	const FUserOnlineAccountAccelByte* User = UsersInfo[0];
	const FText Message = FText::Format(
		TEXT_FORMAT_INVITED,
		FText::FromString(User->GetDisplayName().IsEmpty() ?
			UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(User->GetUserId().Get()) :
			User->GetDisplayName()));

	FString AvatarURL;
	User->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

	PromptSubsystem->PushNotification(
		Message,
		AvatarURL,
		true,
		TEXT_ACCEPT_INVITE,
		TEXT_REJECT_INVITE,
		FText(),
		FPushNotificationDelegate::CreateWeakLambda(
			this,
			[this, Invite, LocalUserNum](EPushNotificationActionResult ActionButtonResult)
			{
				switch (ActionButtonResult)
				{
				case EPushNotificationActionResult::Button1:
					JoinGameSessionConfirmation(LocalUserNum, Invite);
					break;
				case EPushNotificationActionResult::Button2:
					OnlineSession->RejectSessionInvite(LocalUserNum, Invite);
					break;
				default: /* Do nothing */;
				}
			}));
}

void UPlayingWithFriendsSubsystem::OnGameSessionParticipantsChange(
	FName SessionName,
	const FUniqueNetId& Member,
	bool bJoined)
{
	// Abort if not a game session.
	if (!OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
	{
		return;
	}

	// Only handle the event if the game session is in the game server.
	if (!IsMatchSessionGameSessionReceivedServer())
	{
		return;
	}

	UE_LOG_PLAYINGWITHFRIENDS(Verbose, TEXT("Member %s: %s"), *FString(bJoined ? "JOINED" : "LEFT"), *Member.ToDebugString());

	const int32 LocalUserNum = OnlineSession->GetLocalUserNumFromPlayerController(
		OnlineSession->GetPlayerControllerByUniqueNetId(GetSessionOwnerUniqueNetId(SessionName)));
	if (LocalUserNum == INDEX_NONE)
	{
		return;
	}

	TArray<FUniqueNetIdRef> UsersToQuery = {Member.AsShared()};

	// check if leader changed
	if (const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName))
	{
		if (const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo =
			StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo))
		{
			const FUniqueNetIdPtr NewLeaderId = AbSessionInfo->GetLeaderId();
			if (LeaderId.IsValid())
			{
				if (!OnlineSession->CompareAccelByteUniqueId(
					FUniqueNetIdRepl(LeaderId),
					FUniqueNetIdRepl(NewLeaderId)))
				{
					UE_LOG_PLAYINGWITHFRIENDS(VeryVerbose, TEXT("Leader changed to: %s"), *LeaderId->ToDebugString());

					UsersToQuery.Add(NewLeaderId->AsShared());
					bLeaderChanged = true;
				}
			}
			LeaderId = NewLeaderId;
		}
	}

	OnlineSession->QueryUserInfo(
		LocalUserNum,
		UsersToQuery,
		FOnQueryUsersInfoComplete::CreateUObject(
			this,
			&ThisClass::OnQueryUserInfoOnGameSessionParticipantChange,
			SessionName,
			bJoined));
}
