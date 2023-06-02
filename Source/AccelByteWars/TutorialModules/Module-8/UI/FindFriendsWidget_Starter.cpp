// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/UI/FindFriendsWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetList.h"
#include "Components/EditableText.h"
#include "Components/ComboBoxString.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

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

	Cb_SearchType->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSearchTypeChanged);

	Edt_SearchBar->SetText(FText::FromString(TEXT("")));
	Edt_SearchBar->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchBarCommitted);

	WidgetList->ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

void UFindFriendsWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Cb_SearchType->OnSelectionChanged.Clear();
	Edt_SearchBar->OnTextCommitted.Clear();
}

void UFindFriendsWidget_Starter::OnSearchTypeChanged(FString SelectedItem, ESelectInfo::Type SelectInfo)
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

void UFindFriendsWidget_Starter::OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// TODO: Get and display search friend result here.
}

#undef LOCTEXT_NAMESPACE