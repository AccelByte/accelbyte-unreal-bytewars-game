// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendRequestsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

void UFriendRequestsWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);
}

void UFriendRequestsWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// TODO: Bind event to refresh friend request list here.

	GetFriendRequestList();
}

void UFriendRequestsWidget_Starter::NativeOnDeactivated()
{
	// TODO: Unbind event to refresh friend request list here.

	Super::NativeOnDeactivated();
}

void UFriendRequestsWidget_Starter::GetFriendRequestList()
{
	// TODO: Get and display friend request list here.
}