// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlockedPlayersWidget_Starter.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"

void UBlockedPlayersWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem_Starter>();
	ensure(ManagingFriendsSubsystem);
}

void UBlockedPlayersWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset widgets.
	Ws_BlockedPlayers->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_BlockedPlayers->ClearListItems();

	// TODO: Bind event to refresh blocked player list here.

	GetBlockedPlayerList();
}

void UBlockedPlayersWidget_Starter::NativeOnDeactivated()
{
	// TODO: Unbind event to refresh blocked player list here.

	Super::NativeOnDeactivated();
}

void UBlockedPlayersWidget_Starter::GetBlockedPlayerList()
{
	// TODO: Get and display blocked player list here.
}