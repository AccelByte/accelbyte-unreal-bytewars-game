// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-13/UI/BlockedPlayersWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

void UBlockedPlayersWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ManagingFriendsSubsystem = GetGameInstance()->GetSubsystem<UManagingFriendsSubsystem>();
	ensure(ManagingFriendsSubsystem);
}

void UBlockedPlayersWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ManagingFriendsSubsystem->BindOnCachedBlockedPlayersDataUpdated(GetOwningPlayer(), FOnGetCacheBlockedPlayersDataUpdated::CreateUObject(this, &ThisClass::GetBlockedPlayerList));
	GetBlockedPlayerList();
}

void UBlockedPlayersWidget::NativeOnDeactivated()
{
	ManagingFriendsSubsystem->UnbindOnCachedBlockedPlayersDataUpdated(GetOwningPlayer());

	Super::NativeOnDeactivated();
}

void UBlockedPlayersWidget::GetBlockedPlayerList()
{
	ensure(ManagingFriendsSubsystem);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::LoadingEntry);

	ManagingFriendsSubsystem->GetBlockedPlayerList(
		GetOwningPlayer(),
		FOnGetBlockedPlayerListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> BlockedPlayers, const FString& ErrorMessage)
		{
			WidgetList->GetListView()->SetUserFocus(GetOwningPlayer());
			WidgetList->GetListView()->ClearListItems();

			if (bWasSuccessful)
			{
				WidgetList->GetListView()->SetListItems(BlockedPlayers);
				WidgetList->ChangeWidgetListState(BlockedPlayers.IsEmpty() ? EAccelByteWarsWidgetListState::NoEntry : EAccelByteWarsWidgetListState::EntryLoaded);
			}
			else
			{
				WidgetList->SetFailedMessage(FText::FromString(ErrorMessage));
				WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::Error);
			}
		}
	));
}