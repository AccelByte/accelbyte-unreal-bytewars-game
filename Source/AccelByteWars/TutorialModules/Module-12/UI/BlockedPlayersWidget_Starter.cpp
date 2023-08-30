// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-12/UI/BlockedPlayersWidget_Starter.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

void UBlockedPlayersWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem_Starter>();
	ensure(ManagingFriendsSubsystem);
}

void UBlockedPlayersWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

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