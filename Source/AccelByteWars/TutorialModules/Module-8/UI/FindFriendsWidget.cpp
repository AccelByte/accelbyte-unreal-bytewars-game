// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FindFriendsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"
#include "Components/EditableText.h"
#include "Components/ComboBoxString.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

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

	Cb_SearchType->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSearchTypeChanged);

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

void UFindFriendsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Cb_SearchType->OnSelectionChanged.Clear();
	Edt_SearchBar->OnTextCommitted.Clear();
}

void UFindFriendsWidget::OnSearchTypeChanged(FString SelectedItem, ESelectInfo::Type SelectInfo)
{
	int32 SelectedIndex = Cb_SearchType->GetSelectedIndex();

	// Search by user id.
	if (SelectedIndex == 0) 
	{
		Edt_SearchBar->SetHintText(LOCTEXT("Search By User Id", "Search By User Id"));
	}
	// Search by display name.
	else 
	{
		Edt_SearchBar->SetHintText(LOCTEXT("Search By Exact Display Name", "Search By Exact Display Name"));
	}
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
		Cb_SearchType->GetSelectedIndex() == 0 ? ESearchFriendType::ByUserId : ESearchFriendType::ByDisplayName,
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

#undef LOCTEXT_NAMESPACE