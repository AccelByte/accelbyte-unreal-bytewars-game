// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FindFriendsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"
#include "Components/EditableText.h"

void UFindFriendsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void UFindFriendsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

void UFindFriendsWidget::NativeOnDeactivated()
{
	Edt_SearchBar->OnTextCommitted.Clear();

	Super::NativeOnDeactivated();
}

void UFindFriendsWidget::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::Type::OnEnter || Text.IsEmpty())
	{
		return;
	}

	ensure(FriendsSubsystem);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::LoadingEntry);

	FriendsSubsystem->FindFriend(
		GetOwningPlayer(),
		Text.ToString(), 
		FOnFindFriendComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage)
		{
			WidgetList->GetListView()->SetUserFocus(GetOwningPlayer());
			WidgetList->GetListView()->ClearListItems();

			if (bWasSuccessful)
			{
				// Reset the status to be "searched", because the data is retrieved from find friend result.
				FriendData->Status = EFriendStatus::Searched;
				WidgetList->GetListView()->AddItem(FriendData);
				WidgetList->GetListView()->RequestRefresh();
				WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::EntryLoaded);
			}
			else
			{
				WidgetList->SetFailedMessage(FText::FromString(ErrorMessage));
				WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::Error);
			}
		}
	));
}