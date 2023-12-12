// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InviteToGameSessionWidget.h"

#include "CommonButtonBase.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Play/PlayingWithFriends/PlayingWithFriendsSubsystem.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget_Starter.h"

void UInviteToGameSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem>();
	check(Subsystem)

	SetVisibility(Subsystem->IsInMatchSessionGameSession() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Invite->SetIsEnabled(true);
	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::InviteToSession);
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

#pragma region "Helper"
UFriendData* UInviteToGameSessionWidget::GetFriendDataFromParentWidget()
{
	if (const UFriendDetailsWidget* Parent = GetFirstOccurenceOuter<UFriendDetailsWidget>())
	{
		return Parent->GetCachedFriendData();
	}

	if (const UFriendDetailsWidget_Starter* Parent = GetFirstOccurenceOuter<UFriendDetailsWidget_Starter>())
	{
		return Parent->GetCachedFriendData();
	}

	return nullptr;
}
#pragma endregion 

void UInviteToGameSessionWidget::InviteToSession()
{
	const UFriendData* FriendData = GetFriendDataFromParentWidget();
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
