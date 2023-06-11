// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/AccelByteWarsWidgetList.h"
#include "Core/UI/Components/AccelByteWarsListView.h"
#include "Core/UI/Components/AccelByteWarsTileView.h"
#include "Components/WidgetSwitcher.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"

void UAccelByteWarsWidgetList::NativePreConstruct()
{
	Super::NativePreConstruct();

	ListView->SetEntryWidgetClass(EntryWidgetClass);

	TileView->SetEntryWidgetClass(EntryWidgetClass);
	TileView->SetEntryWidth(EntryWidgetWidth);
	TileView->SetEntryHeight(EntryWidgetHeight);

	Ws_ListViewType->SetActiveWidgetIndex((uint8)ListViewType);
}

void UAccelByteWarsWidgetList::NativeConstruct()
{
	Super::NativeConstruct();

	SelectedListViewType = Cast<UListView>(Ws_ListViewType->GetActiveWidget());
	ensure(SelectedListViewType);

	ChangeWidgetListState(EAccelByteWarsWidgetListState::NoEntry);
}

void UAccelByteWarsWidgetList::ChangeWidgetListState(const EAccelByteWarsWidgetListState WidgetListState)
{
	Ws_ListViewState->SetActiveWidgetIndex((uint8)WidgetListState);
}

void UAccelByteWarsWidgetList::SetNoEntryMessage(const FText& Message)
{
	Tb_NoEntryMessage->SetText(Message);
}

void UAccelByteWarsWidgetList::SetFailedMessage(const FText& Message)
{
	Tb_FailedMessage->SetText(Message);
}