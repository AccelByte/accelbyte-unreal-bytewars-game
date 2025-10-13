// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

void UChallengeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance()) 
	{
		ChallengeEssentialsSubsystem = GetGameInstance()->GetSubsystem<UChallengeEssentialsSubsystem>();
	}
	ensure(ChallengeEssentialsSubsystem);
}

// @@@SNIPSTART ChallengeWidget.cpp-NativeOnActivated
void UChallengeWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Set widget title based on the challenge type.
	FString PeriodStr = FAccelByteUtilities::GetUEnumValueAsString(Period).ToLower();
	if (!PeriodStr.IsEmpty()) 
	{
		PeriodStr[0] = FChar::ToUpper(PeriodStr[0]);
	}
	Tb_Title->SetText(
		Period == EAccelByteModelsChallengeRotation::NONE ? 
		ALLTIME_CHALLENGE_TITLE_LABEL : 
		FText::Format(PERIODIC_CHALLENGE_TITLE_LABEL, FText::FromString(PeriodStr)));

	// Bind button events.
	Btn_ClaimAll->SetIsEnabled(false);
	Btn_ClaimAll->OnClicked().AddUObject(this, &ThisClass::OnClaimAllButtonClicked);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Bind the event to update the claim all button state when an individual challenge reward is claimed.
	OnIndividualChallengeRewardsClaimed.BindWeakLambda(this, 
	[this](bool bWasSuccessful, const FString& ErrorMessage)
	{
		if (bWasSuccessful) 
		{
			UpdateClaimAllButton();	
		}
	});

	// Reset list.
	Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty, true);
	Lv_Challenge->ClearListItems();

	GetChallengeGoalList();
}
// @@@SNIPEND

void UChallengeWidget::NativeOnDeactivated()
{
	Btn_ClaimAll->OnClicked().Clear();
	Btn_Back->OnClicked().Clear();

	OnIndividualChallengeRewardsClaimed.Unbind();

	Super::NativeOnDeactivated();
}

UWidget* UChallengeWidget::NativeGetDesiredFocusTarget() const
{
	return Lv_Challenge->GetListItems().IsEmpty() ? Cast<UWidget>(Btn_Back) : Cast<UWidget>(Lv_Challenge);
}

// @@@SNIPSTART ChallengeWidget.cpp-GetChallengeGoalList
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "23-24", "64"]}
void UChallengeWidget::GetChallengeGoalList()
{
	if (!GetOwningPlayer()) 
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. Invalid Player Controller."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!LocalPlayer) 
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. Invalid Local Player."));
		return;
	}

	const FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!UserId) 
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to get challenge goal list. Invalid User ID."));
		return;
	}

	Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	Lv_Challenge->ClearListItems();

	// Get challenge by period.
	ChallengeEssentialsSubsystem->GetChallengeByPeriod(
		Period,
		FOnGetChallengeCodeComplete::CreateWeakLambda(this, [this, UserId](bool bWasSuccessful, const FString& ErrorMessage, const FAccelByteModelsChallenge& Challenge)
		{
			if (!bWasSuccessful)
			{
				Ws_Challenge->ErrorMessage = FText::FromString(ErrorMessage);
				Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
				return;
			}

			// Get and display challenge goal list.
			ChallengeEssentialsSubsystem->GetChallengeGoalList(
				UserId,
				Challenge,
				FOnGetChallengeGoalsComplete::CreateWeakLambda(this, [this]
				(bool bWasSuccessful, const FString& ErrorMessage, const TArray<UChallengeGoalData*>& Goals)
				{
					if (!bWasSuccessful)
					{
						Ws_Challenge->ErrorMessage = FText::FromString(ErrorMessage);
						Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
						return;
					}

					if (Goals.IsEmpty())
					{
						Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
						return;
					}

					Lv_Challenge->SetListItems(Goals);
					Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

					UpdateClaimAllButton();
				}));
		}));
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeWidget.cpp-UpdateClaimAllButton
void UChallengeWidget::UpdateClaimAllButton()
{
	// Collect all claimable reward IDs.
	AllClaimableRewardIds.Empty();
	for (const UObject* Item : Lv_Challenge->GetListItems())
	{
		if (const UChallengeGoalData* GoalData = Cast<UChallengeGoalData>(Item))
		{
			TArray<FString> GoalRewardIds;
			Algo::Transform(GoalData->Progress.ToClaimRewards, GoalRewardIds,
				[](const FAccelByteModelsChallengeClaimableUserReward& Reward) { return Reward.Id; });
			AllClaimableRewardIds.Append(GoalRewardIds);
		}
	}

	// Toggle claim all button based on claimable reward availability.
	Btn_ClaimAll->SetIsEnabled(!AllClaimableRewardIds.IsEmpty());
}
// @@@SNIPEND

// @@@SNIPSTART ChallengeWidget.cpp-OnClaimAllButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-7", "29"]}
void UChallengeWidget::OnClaimAllButtonClicked()
{
	Btn_ClaimAll->SetIsEnabled(false);
	if (AllClaimableRewardIds.IsEmpty()) 
	{
		return;
	}

	GetPromptSubystem()->ShowLoading(CLAIMING_ALL_CHALLENGE_REWARDS_MESSAGE);

	ChallengeEssentialsSubsystem->ClaimChallengeGoalRewards(
		AllClaimableRewardIds,
		FOnClaimChallengeGoalRewardsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& ErrorMessage)
		{
			Btn_ClaimAll->SetIsEnabled(!bWasSuccessful);
			GetPromptSubystem()->HideLoading();

			// If failed, display a push notification to show the error message.
			if (!bWasSuccessful)
			{
				GetPromptSubystem()->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorMessage));
			}
			// If success, refresh the challenge list.
			else
			{
				GetChallengeGoalList();
			}
		}));
}
// @@@SNIPEND

UPromptSubsystem* UChallengeWidget::GetPromptSubystem()
{
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		return GameInstance->GetSubsystem<UPromptSubsystem>();
	}

	return nullptr;
}