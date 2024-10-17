// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RecentPlayersWidget_Starter.h"

#include "Social/RecentPlayers/RecentPlayersLog.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Components/TileView.h"
#include "CommonButtonBase.h"

void URecentPlayersWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();
	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RecentPlayersSubsystem = GameInstance->GetSubsystem<URecentPlayersSubsystem_Starter>();
	ensure(RecentPlayersSubsystem);
}

void URecentPlayersWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	// TODO: Add your Module Recent Players code here.
}

void URecentPlayersWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	// TODO: Add your Module Recent Players code here.
}

UWidget* URecentPlayersWidget_Starter::NativeGetDesiredFocusTarget() const
{
	if (Tv_RecentPlayers->GetListItems().IsEmpty()) 
	{
		return Btn_Back;
	}
	return Tv_RecentPlayers;
}

void URecentPlayersWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

#pragma region Module Recent Players Definitions
// TODO: Add your Module Recent Players code here.
#pragma endregion