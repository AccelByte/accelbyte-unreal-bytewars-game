// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeWidgetEntry.h"
#include "ChallengeGoalRewardWidgetEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "Components/DynamicEntryBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

#include "Algo/ForEach.h"
#include "Algo/MaxElement.h"

void UChallengeWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance())
	{
		ChallengeEssentialsSubsystem = GetGameInstance()->GetSubsystem<UChallengeEssentialsSubsystem>();
	}
	ensure(ChallengeEssentialsSubsystem);

	Cb_ChallengeStatus->SetIsEnabled(false);
	Cb_ChallengeStatus->SetCheckedState(ECheckBoxState::Unchecked);

	Btn_Claim->OnClicked().Clear();
	Btn_Claim->OnClicked().AddUObject(this, &ThisClass::OnClaimButtonClicked);
}

// @@@SNIPSTART ChallengeWidgetEntry.cpp-NativeOnListItemObjectSet
void UChallengeWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	GoalData = Cast<UChallengeGoalData>(ListItemObject);
	if (!GoalData)
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to set challenge widget entry. Invalid data."));
		return;
	}

	const FAccelByteModelsChallengeGoal Goal = GoalData->Goal;
	const FAccelByteModelsChallengeGoalProgress Progress = GoalData->Progress;
	const bool bIsCompleted = Progress.Status == EAccelByteModelsChallengeGoalProgressStatus::COMPLETED;
	const bool bIsRewardsClaimed = Progress.ToClaimRewards.IsEmpty();

	// Display goal basic information.
	Tb_Goal->SetText(FText::FromString(Goal.Name));
	Tb_RemainingTime->SetText(FText::FromString(GoalData->GetEndTimeDuration()));
	Cb_ChallengeStatus->SetCheckedState(bIsCompleted ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	
	// Display rewards
	Deb_Reward->Reset(true);
	for (const FChallengeGoalRewardData& Reward : GoalData->Rewards)
	{
		if (UChallengeGoalRewardWidgetEntry* Entry = Deb_Reward->CreateEntry<UChallengeGoalRewardWidgetEntry>()) 
		{
			Entry->Setup(Reward);
		}
	}

	// Display claim reward button.
	Btn_Claim->SetIsEnabled(!bIsRewardsClaimed);
	Btn_Claim->SetButtonText(bIsRewardsClaimed ? CLAIMED_CHALLENGE_REWARD_LABEL : CLAIMABLE_CHALLENGE_REWARD_LABEL);
	Ws_Progress->SetActiveWidget(bIsCompleted ? Cast<UWidget>(Btn_Claim) : Cast<UWidget>(Hb_Progress));

	/* Select the progress with the highest progress value, as Byte Wars displays only one.
	 * If there is no player progress, set the default goal progress value from the requirement group.
	 * Else, use the actual player progress value. */
	int32 CurrentProgress = 0, TargetProgress = 0;
	if (Progress.Status == EAccelByteModelsChallengeGoalProgressStatus::NOT_STARTED)
	{
		TArray<FAccelByteModelsChallengeGoalRequirementPredicate> Predicates{};
		Algo::ForEach(Progress.Goal.RequirementGroups, [&Predicates](const FAccelByteModelsChallengeGoalRequirement& Group){ Predicates.Append(Group.Predicates); });

		const FAccelByteModelsChallengeGoalRequirementPredicate* Requirement = 
			Algo::MaxElementBy(Predicates, &FAccelByteModelsChallengeGoalRequirementPredicate::TargetValue);
		TargetProgress = Requirement ? Requirement->TargetValue : 0;
	}
	else 
	{
		const FAccelByteModelsChallengeGoalProgressRequirement* Requirement =
			Algo::MaxElementBy(Progress.RequirementProgressions, &FAccelByteModelsChallengeGoalProgressRequirement::CurrentValue);
		CurrentProgress = Requirement ? Requirement->CurrentValue : 0;
		TargetProgress = Requirement ? Requirement->TargetValue : 0;
	}
	Tb_Progress->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentProgress, TargetProgress)));

	OnListItemObjectSet.Broadcast();
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeWidgetEntry.cpp-OnClaimButtonClicked
void UChallengeWidgetEntry::OnClaimButtonClicked()
{
	if (!GoalData || !ChallengeEssentialsSubsystem)
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to claim challenge reward. Invalid data and interface."));
		return;
	}

	// Collect claimable reward IDs.
	TArray<FString> ClaimableRewardIds{};
	Algo::Transform(GoalData->Progress.ToClaimRewards, ClaimableRewardIds, [](const FAccelByteModelsChallengeClaimableUserReward Reward) { return Reward.Id; });

	// Claim rewards.
	Btn_Claim->SetIsEnabled(false);
	ChallengeEssentialsSubsystem->ClaimChallengeGoalRewards(
		ClaimableRewardIds,
		FOnClaimChallengeGoalRewardsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& ErrorMessage)
		{
			Btn_Claim->SetIsEnabled(!bWasSuccessful);
			Btn_Claim->SetButtonText(bWasSuccessful ? CLAIMED_CHALLENGE_REWARD_LABEL : CLAIMABLE_CHALLENGE_REWARD_LABEL);

			// If failed, display a push notification to show the error message.
			if (!bWasSuccessful) 
			{
				GetPromptSubystem()->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorMessage));
			}
			// If success, clear the cached claimable rewards.
			else 
			{
				GoalData->Progress.ToClaimRewards.Empty();
			}
		}));
}
// @@@SNIPEND

UPromptSubsystem* UChallengeWidgetEntry::GetPromptSubystem()
{
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		return GameInstance->GetSubsystem<UPromptSubsystem>();
	}

	return nullptr;
}
