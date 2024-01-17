// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InviteToGameSessionWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Play/PlayingWithFriends/PlayingWithFriendsSubsystem_Starter.h"

void UInviteToGameSessionWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem_Starter>();
	check(Subsystem)

	SetVisibility(Subsystem->IsInMatchSessionGameSession() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Invite->SetIsEnabled(true);
	// TODO: Add your UI delegate setup here
}

void UInviteToGameSessionWidget_Starter::NativeOnDeactivated()
{
	// TODO: Add your UI delegate cleanup here

	if (InviteDelayTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(InviteDelayTimerHandle);
	}

	Super::NativeOnDeactivated();
}

#pragma region "Helper"
UFriendData* UInviteToGameSessionWidget_Starter::GetFriendDataFromParentWidget()
{
	if (const UFriendDetailsWidget* Parent = GetFirstOccurenceOuter<UFriendDetailsWidget>())
	{
		return Parent->GetCachedFriendData();
	}

	return nullptr;
}
#pragma endregion

#pragma region "Playing with Friends implementation"
// TODO: Add your module implementations here.
#pragma endregion 
