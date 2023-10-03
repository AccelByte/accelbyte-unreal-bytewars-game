// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "PlayingWithFriendsSubsystem.h"

#include "PlayingWithFriendsModels.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Interfaces/OnlineUserInterface.h"
#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"

void UPlayingWithFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(OnlineSession))
	{
		return;
	}

	OnlineSession->GetOnJoinSessionCompleteDelegates()->AddUObject(
		this, &ThisClass::OnJoinSessionComplete);
	OnlineSession->GetOnSendSessionInviteCompleteDelegates()->AddUObject(
		this, &ThisClass::OnSendGameSessionInviteComplete);
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

	OnlineSession->GetOnJoinSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSendSessionInviteCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnRejectSessionInviteCompleteDelegate()->RemoveAll(this);
	OnlineSession->GetOnSessionInviteReceivedDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionParticipantsChange()->RemoveAll(this);
}

bool UPlayingWithFriendsSubsystem::IsInMatchSessionGameSession() const
{
	FString RequestedSessionType;
	FNamedOnlineSession* Session = OnlineSession->GetSession(
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

void UPlayingWithFriendsSubsystem::SendGameSessionInvite(const APlayerController* Owner, const FUniqueNetIdPtr Invitee) const
{
	OnlineSession->SendSessionInvite(
		OnlineSession->GetLocalUserNumFromPlayerController(Owner),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Invitee);
}

void UPlayingWithFriendsSubsystem::RejectGameSessionInvite(
	const APlayerController* Owner,
	const FOnlineSessionInviteAccelByte& Invite) const
{
	OnlineSession->RejectSessionInvite(
		OnlineSession->GetLocalUserNumFromPlayerController(Owner),
		Invite);
}

void UPlayingWithFriendsSubsystem::OnSendGameSessionInviteComplete(
	const FUniqueNetId& LocalSenderId,
	FName SessionName,
	bool bSucceeded,
	const FUniqueNetId& InviteeId)
{
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	// show sent notification
	if (UPromptSubsystem* PromptSubsystem = GetPromptSubsystem())
	{
		PromptSubsystem->PushNotification(TEXT_INVITE_SENT);
	}

	OnSendGameSessionInviteCompleteDelegates.Broadcast(LocalSenderId, SessionName, bSucceeded, InviteeId);
}

void UPlayingWithFriendsSubsystem::OnRejectGameSessionInviteComplete(bool bSucceeded) const
{
	OnRejectGameSessionInviteCompleteDelegates.Broadcast(bSucceeded);
}

void UPlayingWithFriendsSubsystem::OnGameSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FOnlineSessionInviteAccelByte& Invite)
{
	// make sure this is a game session
	if (Invite.SessionType != EAccelByteV2SessionType::GameSession)
	{
		return;
	}

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

void UPlayingWithFriendsSubsystem::JoinGameSession(const int32 LocalUserNum, const FOnlineSessionSearchResult& Session) const
{
	OnlineSession->JoinSession(
		LocalUserNum,
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		Session);
}

void UPlayingWithFriendsSubsystem::OnGameSessionParticipantsChange(
	FName SessionName,
	const FUniqueNetId& Member,
	bool bJoined)
{
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	const int32 LocalUserNum = OnlineSession->GetLocalUserNumFromPlayerController(
		OnlineSession->GetPlayerControllerByUniqueNetId(GetSessionOwnerUniqueNetId(SessionName)));
	if (LocalUserNum == INDEX_NONE)
	{
		return;
	}

	OnlineSession->QueryUserInfo(
		LocalUserNum,
		{Member.AsShared()},
		FOnQueryUsersInfoComplete::CreateUObject(
			this,
			&ThisClass::OnQueryUserInfoOnGameSessionParticipantChange,
			SessionName,
			bJoined));
}

void UPlayingWithFriendsSubsystem::OnQueryUserInfoOnGameSessionParticipantChange(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo,
	FName SessionName,
	const bool bJoined)
{
	UPromptSubsystem* PromptSubsystem = GetPromptSubsystem();
	if (!bSucceeded || UsersInfo.IsEmpty() || !PromptSubsystem)
	{
		return;
	}

	const FUserOnlineAccountAccelByte* User = UsersInfo[0];
	const FText Message = FText::Format(
		bJoined ? TEXT_FORMAT_JOINED : TEXT_FORMAT_LEFT,
		FText::FromString(User->GetDisplayName().IsEmpty() ?
			UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(User->GetUserId().Get()) :
			User->GetDisplayName()));

	FString AvatarURL;
	User->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

	PromptSubsystem->PushNotification(Message, AvatarURL);
}

void UPlayingWithFriendsSubsystem::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type CompletionType)
{
	const bool bSucceeded = CompletionType == EOnJoinSessionCompleteResult::Success;
	FText ErrorMessage;

	if (const FNamedOnlineSession* Session = OnlineSession->GetSession(SessionName))
	{
		const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::Cast(*Session->OwningUserId);
	}

	switch (CompletionType)
	{
	case EOnJoinSessionCompleteResult::Success:
		ErrorMessage = FText();
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ErrorMessage = TEXT_FAILED_SESSION_FULL;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ErrorMessage = TEXT_FAILED_SESSION_NULL;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ErrorMessage = TEXT_FAILED_TO_JOIN_SESSION;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ErrorMessage = TEXT_FAILED_ALREADY_IN_SESSION;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		ErrorMessage = TEXT_FAILED_TO_JOIN_SESSION;
		break;
	default:
		ErrorMessage = FText();
	}

	if (UPromptSubsystem* PromptSubsystem = GetPromptSubsystem(); !bSucceeded && PromptSubsystem)
	{
		PromptSubsystem->PushNotification(ErrorMessage);
	}
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
