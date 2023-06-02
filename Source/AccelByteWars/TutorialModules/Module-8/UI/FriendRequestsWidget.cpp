// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendRequestsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

void UFriendRequestsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsEssentialsSubsystem>();
	ensure(FriendsSubsystem);
}

void UFriendRequestsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	FriendsSubsystem->BindOnCachedFriendsDataUpdated(GetOwningPlayer(), FOnCachedFriendsDataUpdated::CreateUObject(this, &ThisClass::GetFriendRequestList));
	GetFriendRequestList();
}

void UFriendRequestsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	FriendsSubsystem->UnbindOnCachedFriendsDataUpdated(GetOwningPlayer());
}

void UFriendRequestsWidget::GetFriendRequestList()
{
	ensure(FriendsSubsystem);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::LoadingEntry);

	FriendsSubsystem->GetInboundFriendRequestList(
		GetOwningPlayer(),
		FOnGetInboundFriendRequestListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> FriendRequests, const FString& ErrorMessage)
		{
			WidgetList->GetListView()->SetUserFocus(GetOwningPlayer());
			WidgetList->GetListView()->ClearListItems();

			if (bWasSuccessful)
			{
				WidgetList->GetListView()->SetListItems(FriendRequests);
				WidgetList->ChangeWidgetListState(FriendRequests.IsEmpty() ? EAccelByteWarsWidgetListState::NoEntry : EAccelByteWarsWidgetListState::EntryLoaded);
			}
			else
			{
				WidgetList->SetFailedMessage(FText::FromString(ErrorMessage));
				WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::Error);
			}
		}
	));
}