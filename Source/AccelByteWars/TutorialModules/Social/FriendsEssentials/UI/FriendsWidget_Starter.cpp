// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsWidget_Starter.h"
#include "FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/TileView.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void UFriendsWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);
}

void UFriendsWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Tv_Friends->ClearListItems();

	Tv_Friends->OnItemClicked().AddUObject(this, &ThisClass::OnFriendEntryClicked);

	// TODO: Bind event to refresh friend list here.

	GetFriendList();
}

void UFriendsWidget_Starter::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	Tv_Friends->OnItemClicked().Clear();

	// TODO: Unbind event to refresh friend list here.

	Super::NativeOnDeactivated();
}

UWidget* UFriendsWidget_Starter::NativeGetDesiredFocusTarget() const
{
	if (Tv_Friends->GetListItems().IsEmpty())
	{
		return Btn_Back;
	}
	return Tv_Friends;
}

void UFriendsWidget_Starter::GetFriendList()
{
	// TODO: Get and display friend list here.
}

void UFriendsWidget_Starter::OnFriendEntryClicked(UObject* Item)
{
	if (!Item)
	{
		UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Unable to handle friend entry on-click event. The friend entry's object item is not valid."));
		return;
	}

	UFriendData* FriendData = Cast<UFriendData>(Item);
	if (!FriendData)
	{
		UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Unable to handle friend entry on-click event. The friend entry's friend data is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Unable to handle friend entry on-click event. Base UI widget is not valid."));
		return;
	}

	UFriendDetailsWidget* DetailsWidget = Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendDetailsWidgetClass));
	if (!DetailsWidget)
	{
		UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Unable to handle friend entry on-click event. Friend details widget is not valid."));
		return;
	}

	DetailsWidget->InitData(FriendData);
}