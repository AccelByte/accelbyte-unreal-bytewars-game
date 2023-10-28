// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsWidget.h"
#include "FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/TileView.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void UFriendsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void UFriendsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Reset widgets.
	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Tv_Friends->ClearListItems();

	Tv_Friends->OnItemClicked().AddUObject(this, &ThisClass::OnFriendEntryClicked);
	
	FriendsSubsystem->BindOnCachedFriendsDataUpdated(GetOwningPlayer(), FOnCachedFriendsDataUpdated::CreateUObject(this, &ThisClass::GetFriendList));
	GetFriendList();
}

void UFriendsWidget::NativeOnDeactivated()
{
	Tv_Friends->OnItemClicked().Clear();

	FriendsSubsystem->UnbindOnCachedFriendsDataUpdated(GetOwningPlayer());

	Super::NativeOnDeactivated();
}

void UFriendsWidget::GetFriendList()
{
	ensure(FriendsSubsystem);

	Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	FriendsSubsystem->GetFriendList(
		GetOwningPlayer(),
		FOnGetFriendListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage)
		{
			Tv_Friends->SetUserFocus(GetOwningPlayer());
			Tv_Friends->ClearListItems();

			if (bWasSuccessful) 
			{
				Tv_Friends->SetListItems(Friends);
				Ws_Friends->SetWidgetState(Friends.IsEmpty() ?
					EAccelByteWarsWidgetSwitcherState::Empty :
					EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
			else
			{
				Ws_Friends->ErrorMessage = FText::FromString(ErrorMessage);
				Ws_Friends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
			}
		}
	));
}

void UFriendsWidget::OnFriendEntryClicked(UObject* Item) 
{
	UFriendData* FriendData = Cast<UFriendData>(Item);
	ensure(FriendData);

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	UFriendDetailsWidget* DetailsWidget = Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendDetailsWidgetClass));
	ensure(DetailsWidget);

	DetailsWidget->InitData(FriendData);
}