// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
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

	FString PeriodStr = FAccelByteUtilities::GetUEnumValueAsString(Period).ToLower();
	if (!PeriodStr.IsEmpty()) 
	{
		PeriodStr[0] = FChar::ToUpper(PeriodStr[0]);
	}
	Tb_Title->SetText(
		Period == EAccelByteModelsChallengeRotation::NONE ? 
		ALLTIME_CHALLENGE_TITLE_LABEL : 
		FText::Format(PERIODIC_CHALLENGE_TITLE_LABEL, FText::FromString(PeriodStr)));
	
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

	// Reset list.
	Ws_Challenge->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty, true);
	Lv_Challenge->ClearListItems();

	GetChallengeGoalList();
}
// @@@SNIPEND

void UChallengeWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UChallengeWidget::NativeGetDesiredFocusTarget() const
{
	return Lv_Challenge->GetListItems().IsEmpty() ? Cast<UWidget>(Btn_Back) : Cast<UWidget>(Lv_Challenge);
}

// @@@SNIPSTART ChallengeWidget.cpp-GetChallengeGoalList
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "23-24", "62"]}
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
				}));
		}));
}
// @@@SNIPEND