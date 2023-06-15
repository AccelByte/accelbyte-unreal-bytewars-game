// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/SentFriendRequestsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"

void USentFriendRequestsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void USentFriendRequestsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	FriendsSubsystem->BindOnCachedFriendsDataUpdated(GetOwningPlayer(), FOnCachedFriendsDataUpdated::CreateUObject(this, &ThisClass::GetSentFriendRequestList));
	GetSentFriendRequestList();
}

void USentFriendRequestsWidget::NativeOnDeactivated()
{
	FriendsSubsystem->UnbindOnCachedFriendsDataUpdated(GetOwningPlayer());

	Super::NativeOnDeactivated();
}

void USentFriendRequestsWidget::GetSentFriendRequestList()
{
	ensure(FriendsSubsystem);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::LoadingEntry);

	FriendsSubsystem->GetOutboundFriendRequestList(
		GetOwningPlayer(),
		FOnGetOutboundFriendRequestListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage)
		{
			WidgetList->GetListView()->SetUserFocus(GetOwningPlayer());
			WidgetList->GetListView()->ClearListItems();

			if (bWasSuccessful)
			{
				WidgetList->GetListView()->SetListItems(Friends);
				WidgetList->ChangeWidgetListState(Friends.IsEmpty() ? EAccelByteWarsWidgetListState::NoEntry : EAccelByteWarsWidgetListState::EntryLoaded);
			}
			else
			{
				WidgetList->SetFailedMessage(FText::FromString(ErrorMessage));
				WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::Error);
			}
		}
	));
}