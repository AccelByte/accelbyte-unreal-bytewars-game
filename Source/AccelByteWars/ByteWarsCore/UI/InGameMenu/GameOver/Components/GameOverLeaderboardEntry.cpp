// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/InGameMenu/GameOver/Components/GameOverLeaderboardEntry.h"
#include "Components/TextBlock.h"

void UGameOverLeaderboardEntry::InitData(const FText& PlayerName, const int32 PlayerScore, const int32 PlayerKills, const FLinearColor& PlayerColor)
{
	Txt_PlayerName->SetText(PlayerName);
	Txt_PlayerScore->SetText(FText::FromString(FString::Printf(TEXT("%d"), PlayerScore)));
	Txt_PlayerKills->SetText(FText::FromString(FString::Printf(TEXT("%d"), PlayerKills)));

	Txt_PlayerName->SetColorAndOpacity(PlayerColor);
	Txt_PlayerScore->SetColorAndOpacity(PlayerColor);
	Txt_PlayerKills->SetColorAndOpacity(PlayerColor);
}