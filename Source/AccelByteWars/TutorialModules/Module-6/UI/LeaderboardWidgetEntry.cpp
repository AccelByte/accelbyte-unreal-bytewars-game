// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-6/UI/LeaderboardWidgetEntry.h"
#include "TutorialModules/Module-6/LeaderboardEssentialsModels.h"
#include "Components/TextBlock.h"

void ULeaderboardWidgetEntry::SetLeaderboardRank(ULeaderboardRank* LeaderboardRank)
{
	NativeOnListItemObjectSet(LeaderboardRank);
}

void ULeaderboardWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	const ULeaderboardRank* LeaderboardRank = Cast<ULeaderboardRank>(ListItemObject);
	if (!LeaderboardRank) 
	{
		return;
	}

	// Display the player's rank. If no rank, display it as #?.
	Tb_Rank->SetText(FText::FromString(FString::Printf(TEXT("#%s"), 
		LeaderboardRank->Rank < 0 ? TEXT("?") : *FString::FromInt(LeaderboardRank->Rank))));

	// Display the player's display name.
	Tb_DisplayName->SetText(FText::FromString(LeaderboardRank->DisplayName));

	// Display the player's score. If no score, display it as empty.
	Tb_Score->SetText(FText::FromString(FString::Printf(TEXT("%s"),
		LeaderboardRank->Score < 0 ? TEXT("") : *FString::FromInt((int32)LeaderboardRank->Score))));
}
