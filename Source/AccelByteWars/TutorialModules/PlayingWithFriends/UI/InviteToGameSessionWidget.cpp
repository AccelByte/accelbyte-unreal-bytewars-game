// Fill out your copyright notice in the Description page of Project Settings.


#include "InviteToGameSessionWidget.h"

#include "CommonButtonBase.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "TutorialModules/PlayingWithFriends/PlayingWithFriendsSubsystem.h"

void UInviteToGameSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem>();
	check(Subsystem)

	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::InviteToSession);
	Btn_Invite->SetIsEnabled(true);

	SetVisibility(Subsystem->IsInMatchSessionGameSession() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UInviteToGameSessionWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_Invite->OnClicked().RemoveAll(this);

	if (InviteDelayTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(InviteDelayTimerHandle);
	}
}

void UInviteToGameSessionWidget::InviteToSession()
{
	const UFriendDetailsWidget* Parent = GetFirstOccurenceOuter<UFriendDetailsWidget>();
	if (!Parent)
	{
		return;
	}

	const UFriendData* FriendData = Parent->GetCachedFriendData();
	if (!FriendData)
	{
		return;
	}

	Subsystem->SendGameSessionInvite(GetOwningPlayer(), FriendData->UserId);

	// disable button for 5 seconds to avoid spamming
	Btn_Invite->SetIsEnabled(false);
	GetGameInstance()->GetTimerManager().SetTimer(
		InviteDelayTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Btn_Invite->SetIsEnabled(true);
		}),
		5.0f,
		false,
		5.0f);
}
