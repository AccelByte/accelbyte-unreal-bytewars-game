// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengePeriodWidget.h"
#include "ChallengeWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void UChallengePeriodWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Alltime->OnClicked().AddUObject(this, &ThisClass::OpenChallengeMenu, EAccelByteModelsChallengeRotation::NONE);
	Btn_Daily->OnClicked().AddUObject(this, &ThisClass::OpenChallengeMenu, EAccelByteModelsChallengeRotation::DAILY);
	Btn_Weekly->OnClicked().AddUObject(this, &ThisClass::OpenChallengeMenu, EAccelByteModelsChallengeRotation::WEEKLY);

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
}

void UChallengePeriodWidget::NativeOnDeactivated()
{
	Btn_Alltime->OnClicked().Clear();
	Btn_Daily->OnClicked().Clear();
	Btn_Weekly->OnClicked().Clear();

	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UChallengePeriodWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_Back;
}

void UChallengePeriodWidget::OpenChallengeMenu(const EAccelByteModelsChallengeRotation Period)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance) 
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to open challenge menu. Invalid game instance."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget) 
	{
		UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to open challenge menu. Invalid base UI widget."));
		return;
	}

	BaseUIWidget->PushWidgetToStack(
		EBaseUIStackType::Menu,
		ChallengeWidgetClass.Get(),
		[Period](UAccelByteWarsActivatableWidget& Widget)
		{
			if (UChallengeWidget* ChallengeWidget = Cast<UChallengeWidget>(&Widget))
			{
				ChallengeWidget->Period = Period;
				return;
			}

			UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("Failed to open challenge menu. Invalid widget class."));
		});
}
