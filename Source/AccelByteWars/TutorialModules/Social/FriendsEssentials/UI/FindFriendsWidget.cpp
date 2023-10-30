// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FindFriendsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"
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

	// Reset widgets.
	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FindFriends->ClearListItems();
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

	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	FriendsSubsystem->FindFriend(
		GetOwningPlayer(),
		Text.ToString(), 
		FOnFindFriendComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage)
		{
			Lv_FindFriends->SetUserFocus(GetOwningPlayer());
			Lv_FindFriends->ClearListItems();

			if (bWasSuccessful)
			{
				// Reset the status to be "searched", because the data is retrieved from find friend result.
				FriendData->Status = EFriendStatus::Searched;
				Lv_FindFriends->AddItem(FriendData);
				Lv_FindFriends->RequestRefresh();
				Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
			}
			else
			{
				Ws_FindFriends->ErrorMessage = FText::FromString(ErrorMessage);
				Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
			}
		}
	));
}