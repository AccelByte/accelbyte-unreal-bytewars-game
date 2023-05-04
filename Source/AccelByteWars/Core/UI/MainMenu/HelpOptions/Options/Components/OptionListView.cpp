// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Options/Components/OptionListView.h"

void UOptionListView::AddNameOverride(const FName& DevName, const FText& OverrideName)
{
	NameOverrides.Add(DevName, OverrideName);
}