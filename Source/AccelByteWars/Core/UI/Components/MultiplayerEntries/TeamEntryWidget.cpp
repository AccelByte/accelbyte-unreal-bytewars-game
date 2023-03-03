// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/MultiplayerEntries/TeamEntryWidget.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Components/HorizontalBox.h"

void UTeamEntryWidget::AddPlayerEntry(UPlayerEntryWidget* InPlayerEntry)
{
	Hb_TeamList->AddChild(InPlayerEntry);
}

void UTeamEntryWidget::ClearPlayerEntries()
{
	Hb_TeamList->ClearChildren();
}