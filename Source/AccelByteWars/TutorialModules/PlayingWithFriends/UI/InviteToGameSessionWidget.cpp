// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InviteToGameSessionWidget.h"

#include "CommonButtonBase.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "TutorialModules/PlayingWithFriends/PlayingWithFriendsSubsystem.h"

void UInviteToGameSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem>();
	check(Subsystem)

	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::InviteToSession);
	Btn_Invite->SetIsEnabled(true);

	SetVisibility(Subsystem->IsInMatchSessionGameSession() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UInviteToGameSessionWidget::NativeOnDeactivated()
{
	Btn_Invite->OnClicked().RemoveAll(this);

	if (InviteDelayTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(InviteDelayTimerHandle);
	}

	Super::NativeOnDeactivated();
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
