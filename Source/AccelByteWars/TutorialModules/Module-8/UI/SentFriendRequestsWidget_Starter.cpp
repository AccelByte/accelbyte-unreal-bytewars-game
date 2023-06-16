// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/SentFriendRequestsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

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

	// TODO: Bind event to refresh sent friend request list here.

	GetSentFriendRequestList();
}

void USentFriendRequestsWidget_Starter::NativeOnDeactivated()
{
	// TODO: Unbind event to refresh sent friend request list here.

	Super::NativeOnDeactivated();
}

void USentFriendRequestsWidget_Starter::GetSentFriendRequestList()
{
	// TODO: Get and display sent friend request list here.
}