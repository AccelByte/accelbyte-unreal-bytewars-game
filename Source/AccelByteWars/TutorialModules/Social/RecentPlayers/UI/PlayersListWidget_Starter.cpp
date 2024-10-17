// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PlayersListWidget_Starter.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/RecentPlayers/RecentPlayersLog.h"

#include "Components/TileView.h"

void UPlayersListWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();
	
	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RecentPlayersSubsystem = GameInstance->GetSubsystem<URecentPlayersSubsystem_Starter>();
	ensure(RecentPlayersSubsystem);
}

void UPlayersListWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// TODO: Add your Module Recent Players code here.
}

void UPlayersListWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// TODO: Add your Module Recent Players code here.
}

UWidget* UPlayersListWidget_Starter::NativeGetDesiredFocusTarget() const
{
	if (Tv_PlayersList->GetListItems().IsEmpty()) 
	{
		return Btn_Back;
	}
	return Tv_PlayersList;
}

#pragma region Module Recent Players Definitions
// TODO: Add your Module Recent Players code here.
#pragma endregion