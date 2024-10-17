// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsSyncWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/NativeFriendsEssentials/NativeFriendsSubsystem.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UFriendsSyncWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);

	NativeFriendsSubsystem = GameInstance->GetSubsystem<UNativeFriendsSubsystem>();
	ensure(NativeFriendsSubsystem);

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem>();
	ensure(ManagingFriendsSubsystem);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UFriendsSyncWidget::NativeOnActivated()
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

void UFriendsSyncWidget::NativeOnDeactivated()
{
	Btn_ExecuteFriendSync->OnClicked().Clear();
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UFriendsSyncWidget::NativeGetDesiredFocusTarget() const
{
	return Lv_Friends->GetListItems().IsEmpty() ? static_cast<UWidget*>(Btn_Back) : static_cast<UWidget*>(Lv_Friends);
}

void UFriendsSyncWidget::FetchNativeFriendList()
{
	PromptSubsystem->ShowLoading(FETCH_NATIVE_FRIENDS_MESSAGE);

	// Populate AccelByte friendlist cache
	FriendsSubsystem->GetFriendList(
		GetOwningPlayer(),
		FOnGetFriendListComplete::CreateUObject(this, &ThisClass::OnGetFriendListComplete));
}

void UFriendsSyncWidget::OnGetFriendListComplete(bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage)
{
	if (bWasSuccessful)
	{
		// Populate AccelByte blocked friendlist cache
		ManagingFriendsSubsystem->GetBlockedPlayerList(
			GetOwningPlayer(),
			true,
			FOnGetBlockedPlayerListComplete::CreateUObject(this, &ThisClass::OnGetBlockedPlayerListCompelete));
	}
	else
	{
		Ws_Friends->ErrorMessage = FText::FromString(ErrorMessage);
		Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);

		PromptSubsystem->HideLoading();
	}
}

void UFriendsSyncWidget::OnGetBlockedPlayerListCompelete(bool bWasSuccessful, TArray<UFriendData*> BlockedPlayers, const FString& ErrorMessage)
{
	if (bWasSuccessful)
	{
		NativeFriendsSubsystem->GetNativeFriendList(
			GetOwningPlayer(),
			FOnGetNativeFriendListComplete::CreateUObject(this, &ThisClass::OnGetNativeFriendListComplete));
	}
	else
	{
		Ws_Friends->ErrorMessage = FText::FromString(ErrorMessage);
		Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);

		PromptSubsystem->HideLoading();
	}
}

void UFriendsSyncWidget::OnGetNativeFriendListComplete(bool bWasSuccessful, TArray<UNativeFriendData*> Friends, const FString& ErrorMessage)
{
	Lv_Friends->ClearListItems();
	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);

	if (bWasSuccessful)
	{
		Lv_Friends->SetListItems(Friends);
		Ws_Friends->SetWidgetState(Friends.IsEmpty() ?
			EAccelByteWarsWidgetSwitcherState::Empty :
			EAccelByteWarsWidgetSwitcherState::Not_Empty);
	}
	else
	{
		Ws_Friends->ErrorMessage = FText::FromString(ErrorMessage);
		Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}

	PromptSubsystem->HideLoading();
}

void UFriendsSyncWidget::SyncNativePlatformFriendList()
{
	PromptSubsystem->ShowLoading(SYNC_IN_PROGRESS_MESSAGE);

	// Reset widgets.
	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_Friends->ClearListItems();
	Tb_SyncResult->SetText(FText());

	NativeFriendsSubsystem->SyncNativePlatformFriendList(
		GetOwningPlayer(),
		FOnSyncNativePlatformFriendListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& ErrorMessage)
		{
			PromptSubsystem->HideLoading();

			if (bWasSuccessful)
			{
				Tb_SyncResult->SetText(FRIEND_SYNCED_MESSAGE);
				Tb_SyncResult->SetColorAndOpacity(FColor::Green);
				FetchNativeFriendList();
			}
			else
			{
				Tb_SyncResult->SetText(FText::Format(SYNC_FAILED_MESSAGE, FText::FromString(ErrorMessage)));
				Tb_SyncResult->SetColorAndOpacity(FColor::Red);
			}
		})
	);
}
