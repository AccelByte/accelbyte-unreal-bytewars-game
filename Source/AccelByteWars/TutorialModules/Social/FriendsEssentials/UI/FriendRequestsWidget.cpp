// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendRequestsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"

void UFriendRequestsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void UFriendRequestsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset widgets.
	Ws_FriendRequests->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FriendRequests->ClearListItems();

	FriendsSubsystem->BindOnCachedFriendsDataUpdated(GetOwningPlayer(), FOnCachedFriendsDataUpdated::CreateUObject(this, &ThisClass::GetFriendRequestList));
	GetFriendRequestList();
}

void UFriendRequestsWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	FriendsSubsystem->UnbindOnCachedFriendsDataUpdated(GetOwningPlayer());

	Super::NativeOnDeactivated();
}

UWidget* UFriendRequestsWidget::NativeGetDesiredFocusTarget() const
{
	if (Lv_FriendRequests->GetListItems().IsEmpty())
	{
		return Btn_Back;
	}
	return Lv_FriendRequests;
}

void UFriendRequestsWidget::GetFriendRequestList()
{
	ensure(FriendsSubsystem);

	Ws_FriendRequests->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	FriendsSubsystem->GetInboundFriendRequestList(
		GetOwningPlayer(),
		FOnGetInboundFriendRequestListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> FriendRequests, const FString& ErrorMessage)
		{
			Lv_FriendRequests->ClearListItems();

			if (bWasSuccessful)
			{
				Lv_FriendRequests->SetListItems(FriendRequests);
				Ws_FriendRequests->SetWidgetState(FriendRequests.IsEmpty() ?
					EAccelByteWarsWidgetSwitcherState::Empty :
					EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
			else
			{
				Ws_FriendRequests->ErrorMessage = FText::FromString(ErrorMessage);
				Ws_FriendRequests->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
			}
		}
	));
}