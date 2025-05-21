// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendWidgetEntry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OnlineSubsystemUtils.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem.h"
#include "Social/RecentPlayers/RecentPlayersSubsystem_Starter.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	FriendsSubsystem = GetGameInstance()->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);

	Btn_Invite->OnClicked().Clear();
	Btn_Accept->OnClicked().Clear();
	Btn_Reject->OnClicked().Clear();
	Btn_Cancel->OnClicked().Clear();

	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::OnInviteButtonClicked);
	Btn_Accept->OnClicked().AddUObject(this, &ThisClass::OnAcceptButtonClicked);
	Btn_Reject->OnClicked().AddUObject(this, &ThisClass::OnRejectButtonClicked);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::OnCancelButtonClicked);
}

// @@@SNIPSTART FriendWidgetEntry.cpp-NativeOnListItemObjectSet
// @@@MULTISNIP PlayerGameSessionStatus {"highlightedLines": "{22-48}"}
void UFriendWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	CachedFriendData = Cast<UFriendData>(ListItemObject);

	// Display display name.
	if (!CachedFriendData->DisplayName.IsEmpty()) 
	{
		Tb_DisplayName->SetText(FText::FromString(CachedFriendData->DisplayName));
	}
	else 
	{
		Tb_DisplayName->SetText(FText::FromString(
			UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(CachedFriendData->UserId.ToSharedRef().Get())));
	}

	// Display avatar image.
	const FString AvatarURL = CachedFriendData->AvatarURL;
	Img_Avatar->LoadImage(AvatarURL);

	if(CachedFriendData->DataSource != EDataSource::User)
	{
		Ws_OptionButtons->SetVisibility(ESlateVisibility::Hidden);
		if(CachedFriendData->DataSource == EDataSource::GameSessionPlayer)
		{
			URecentPlayersSubsystem* RecentPlayersSubsystem = GetGameInstance()->GetSubsystem<URecentPlayersSubsystem>();
			if(RecentPlayersSubsystem != nullptr)
			{
				Tb_GameSessionStatus->SetVisibility(ESlateVisibility::Visible);
				Tb_GameSessionStatus->SetText(FText::FromString(RecentPlayersSubsystem->GetGameSessionPlayerStatus(CachedFriendData)));

				const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
				IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

				if(SessionInterface.IsValid())
				{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
					SessionInterface->ClearOnSessionParticipantJoinedDelegates(this);
					SessionInterface->AddOnSessionParticipantJoinedDelegate_Handle(FOnSessionParticipantJoinedDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsJoined));
#else
					SessionInterface->ClearOnSessionParticipantsChangeDelegates(this);
					SessionInterface->AddOnSessionParticipantsChangeDelegate_Handle(FOnSessionParticipantsChangeDelegate::CreateUObject(this, &ThisClass::OnSessionParticipantsChanged));
#endif
				}
			}
		}
	}
	else
	{
		// Display options based on friend's invitation status.
		Ws_OptionButtons->SetActiveWidgetIndex((uint8)CachedFriendData->Status);

		// Show the reason why the player cannot send invitation request.
		Btn_Invite->SetVisibility(!CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Tb_CannotInviteMessage->SetVisibility(CachedFriendData->bCannotBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Tb_CannotInviteMessage->SetText(FText::FromString(CachedFriendData->ReasonCannotBeInvited));

		Tb_GameSessionStatus->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnListItemObjectSet.Broadcast();
}
// @@@SNIPEND

// @@@SNIPSTART FriendWidgetEntry.cpp-OnInviteButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "20"]}
void UFriendWidgetEntry::OnInviteButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->SendFriendRequest(
		GetOwningPlayer(), 
		CachedFriendData->UserId, 
		FOnSendFriendRequestComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage) 
		{
			if (bWasSuccessful) 
			{
				// Since the invitation is already sent, refresh the entry data to show that the friend cannot be invited again.
				CachedFriendData->bCannotBeInvited = FriendData->bCannotBeInvited;
				CachedFriendData->ReasonCannotBeInvited = FriendData->ReasonCannotBeInvited;
				NativeOnListItemObjectSet(CachedFriendData);
			}
		}
	));
}
// @@@SNIPEND

// @@@SNIPSTART FriendWidgetEntry.cpp-OnAcceptButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "7"]}
void UFriendWidgetEntry::OnAcceptButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->AcceptFriendRequest(GetOwningPlayer(), CachedFriendData->UserId);
}
// @@@SNIPEND

// @@@SNIPSTART FriendWidgetEntry.cpp-OnRejectButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "7"]}
void UFriendWidgetEntry::OnRejectButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	FriendsSubsystem->RejectFriendRequest(GetOwningPlayer(), CachedFriendData->UserId);
}
// @@@SNIPEND

// @@@SNIPSTART FriendWidgetEntry.cpp-OnCancelButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "8"]}
void UFriendWidgetEntry::OnCancelButtonClicked()
{
	ensure(CachedFriendData);
	ensure(FriendsSubsystem);

	// Cancel friend request is the same as removing a friend.
	FriendsSubsystem->CancelFriendRequest(GetOwningPlayer(), CachedFriendData->UserId);
}
// @@@SNIPEND

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
void UFriendWidgetEntry::OnSessionParticipantsJoined(FName SessionName, const FUniqueNetId& User)
{
	if (!SessionName.IsEqual(NAME_GameSession) || User != *CachedFriendData->UserId.Get())
	{
		return;
	}
	NativeOnListItemObjectSet(CachedFriendData);
}
#else
void UFriendWidgetEntry::OnSessionParticipantsChanged(FName SessionName, const FUniqueNetId& User, bool bJoined)
{
	if(!SessionName.IsEqual(NAME_GameSession) || User != *CachedFriendData->UserId.Get())
	{
		return;
	}
	NativeOnListItemObjectSet(CachedFriendData);
}
#endif
#undef LOCTEXT_NAMESPACE
