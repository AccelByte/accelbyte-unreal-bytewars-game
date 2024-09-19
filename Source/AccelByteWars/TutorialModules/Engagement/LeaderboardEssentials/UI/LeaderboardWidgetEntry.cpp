// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/LeaderboardEssentials/UI/LeaderboardWidgetEntry.h"
#include "Engagement/LeaderboardEssentials/LeaderboardEssentialsModels.h"
#include "Components/TextBlock.h"

void ULeaderboardWidgetEntry::SetLeaderboardRank(ULeaderboardRank* LeaderboardRank)
{
	NativeOnListItemObjectSet(LeaderboardRank);
}

// @@@SNIPSTART LeaderboardWidgetEntry.cpp-NativeOnListItemObjectSet
void ULeaderboardWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	const ULeaderboardRank* LeaderboardRank = Cast<ULeaderboardRank>(ListItemObject);
	if (!LeaderboardRank) 
	{
		return;
	}

	const bool bUnranked = LeaderboardRank->Rank <= 0;

	// Display the player's rank. If no rank, display it as #?.
	Tb_Rank->SetText(bUnranked ? FText::FromString(TEXT("?")) : FText::AsNumber(LeaderboardRank->Rank));

	// Display the player's display name.
	Tb_DisplayName->SetText(FText::FromString(LeaderboardRank->DisplayName));

	// Display the player's score. If no rank, display it as empty.
	Tb_Score->SetText(bUnranked ? FText::FromString(TEXT("")) : FText::AsNumber(LeaderboardRank->Score));
}
// @@@SNIPEND