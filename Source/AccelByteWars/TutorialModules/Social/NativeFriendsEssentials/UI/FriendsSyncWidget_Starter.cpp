// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsSyncWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Social/FriendsEssentials/FriendsSubsystem_Starter.h"
#include "Social/NativeFriendsEssentials/NativeFriendsSubsystem_Starter.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem_Starter.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UFriendsSyncWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	NativeFriendsSubsystem = GameInstance->GetSubsystem<UNativeFriendsSubsystem_Starter>();
	ensure(NativeFriendsSubsystem);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	if (!ensure(FriendsSubsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("FriendsSubsystem_Starter is invalid. Make sure Friend Essentials module and its starter mode are active."))
		return;
	}

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem_Starter>();
	if (!ensure(ManagingFriendsSubsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("ManagingFriendsSubsystem_Starter is invalid. Make sure Managing Friends module and its starter mode are active."))
		return;
	}

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UFriendsSyncWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_ExecuteFriendSync->OnClicked().AddUObject(this, &ThisClass::SyncNativePlatformFriendList);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_Friends->ClearListItems();
	Tb_SyncResult->SetText(FText());

	FetchNativeFriendList();
}

void UFriendsSyncWidget_Starter::NativeOnDeactivated()
{
	Btn_ExecuteFriendSync->OnClicked().Clear();
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UFriendsSyncWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Lv_Friends->GetListItems().IsEmpty() ? static_cast<UWidget*>(Btn_Back) : static_cast<UWidget*>(Lv_Friends);
}

void UFriendsSyncWidget_Starter::FetchNativeFriendList()
{
	// TODO: Get and display native friend list here.
}

void UFriendsSyncWidget_Starter::SyncNativePlatformFriendList()
{
	// TODO: Get and display native friend list here.
}
