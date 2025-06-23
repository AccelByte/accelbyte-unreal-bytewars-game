// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeGoalRewardWidgetEntry.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Components/TextBlock.h"

// @@@SNIPSTART ChallengeGoalRewardWidgetEntry.cpp-Setup
void UChallengeGoalRewardWidgetEntry::Setup(const FChallengeGoalRewardData& RewardData) const
{
	Img_Reward->LoadImage(RewardData.IconUrl);
	Tb_RewardValue->SetText(FText::FromString(FString::Printf(TEXT("%d"), RewardData.Quantity)));
}
// @@@SNIPEND