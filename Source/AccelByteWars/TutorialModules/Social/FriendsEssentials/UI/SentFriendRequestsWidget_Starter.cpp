// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SentFriendRequestsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"

void USentFriendRequestsWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);
}

void USentFriendRequestsWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_FriendRequests->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FriendRequests->ClearListItems();

	// TODO: Bind event to refresh sent friend request list here.

	GetSentFriendRequestList();
}

void USentFriendRequestsWidget_Starter::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	// TODO: Unbind event to refresh sent friend request list here.

	Super::NativeOnDeactivated();
}

UWidget* USentFriendRequestsWidget_Starter::NativeGetDesiredFocusTarget() const
{
	if (Lv_FriendRequests->GetListItems().IsEmpty())
	{
		return Btn_Back;
	}
	return Lv_FriendRequests;
}

void USentFriendRequestsWidget_Starter::GetSentFriendRequestList()
{
	// TODO: Get and display sent friend request list here.
}