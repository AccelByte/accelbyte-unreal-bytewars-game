// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FindFriendsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/ListView.h"
#include "Components/EditableText.h"

void UFindFriendsWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem_Starter>();
	ensure(FriendsSubsystem);
}

void UFindFriendsWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);

	// Reset widgets.
	Ws_FindFriends->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	Lv_FindFriends->ClearListItems();
}

void UFindFriendsWidget_Starter::NativeOnDeactivated()
{
	Edt_SearchBar->OnTextCommitted.Clear();

	Super::NativeOnDeactivated();
}

void UFindFriendsWidget_Starter::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// TODO: Get and display find friend result here.
}