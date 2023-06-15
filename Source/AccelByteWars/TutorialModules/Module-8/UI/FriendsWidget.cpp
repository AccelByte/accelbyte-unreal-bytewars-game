// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FriendsWidget.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"
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

	WidgetList->GetListView()->OnItemClicked().AddUObject(this, &ThisClass::OnFriendEntryClicked);
	
	FriendsSubsystem->BindOnCachedFriendsDataUpdated(GetOwningPlayer(), FOnCachedFriendsDataUpdated::CreateUObject(this, &ThisClass::GetFriendList));
	GetFriendList();
}

void UFriendsWidget::NativeOnDeactivated()
{
	WidgetList->GetListView()->OnItemClicked().Clear();

	FriendsSubsystem->UnbindOnCachedFriendsDataUpdated(GetOwningPlayer());

	Super::NativeOnDeactivated();
}

void UFriendsWidget::GetFriendList()
{
	ensure(FriendsSubsystem);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::LoadingEntry);

	FriendsSubsystem->GetFriendList(
		GetOwningPlayer(),
		FOnGetFriendListComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage)
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