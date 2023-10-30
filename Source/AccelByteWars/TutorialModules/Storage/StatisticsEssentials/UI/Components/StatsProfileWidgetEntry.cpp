// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsProfileWidgetEntry.h"

#include "Components/TextBlock.h"

void UStatsProfileWidgetEntry::Setup(FText StatCode, FText StatValue) const
{
	Txt_StatName->SetText(StatCode);
	Txt_StatValue->SetText(StatValue);
}
