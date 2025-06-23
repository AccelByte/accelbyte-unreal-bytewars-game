// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "TutorialModules/Engagement/ChallengeEssentials/ChallengeEssentialsModels.h"
#include "ChallengeGoalRewardWidgetEntry.generated.h"

class UAccelByteWarsAsyncImageWidget;
class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UChallengeGoalRewardWidgetEntry : public UUserWidget
{
	GENERATED_BODY()

// @@@SNIPSTART ChallengeGoalRewardWidgetEntry.h-public
// @@@MULTISNIP Setup {"selectedLines": ["1-2"]}
public:
	void Setup(const FChallengeGoalRewardData& RewardData) const;
// @@@SNIPEND

// @@@SNIPSTART ChallengeGoalRewardWidgetEntry.h-protected
// @@@MULTISNIP ChallengeGoalRewardEntryUI {"selectedLines": ["1", "4-8"]}
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_RewardValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* Img_Reward;
// @@@SNIPEND
};
