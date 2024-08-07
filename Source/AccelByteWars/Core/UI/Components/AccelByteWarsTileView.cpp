// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/AccelByteWarsTileView.h"

void UAccelByteWarsTileView::SetEntryWidgetClass(TSubclassOf<UAccelByteWarsWidgetEntry>& InEntryWidgetClass)
{
	EntryWidgetClass = InEntryWidgetClass;
	RequestRefresh();
}