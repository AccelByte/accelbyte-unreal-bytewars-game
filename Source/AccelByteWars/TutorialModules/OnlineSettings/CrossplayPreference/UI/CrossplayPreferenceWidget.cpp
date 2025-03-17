// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CrossplayPreferenceWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/MainMenu/HelpOptions/Options/Components/OptionListEntryBase.h"
#include "OnlineSettings/CrossplayPreference/CrossplayPreferenceLog.h"
#include "OnlineSettings/CrossplayPreference/CrossplayPreferenceSubsystem.h"

void UCrossplayPreferenceWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::UpdateCrossplayPreference);

	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	Subsystem = GameInstance->GetSubsystem<UCrossplayPreferenceSubsystem>();
	ensure(GameInstance);

	ResetUI();

	const FUniqueNetIdPtr OwningPlayerNetId = AccelByteWarsUtility::GetUserId(GetOwningPlayer());
	if (!OwningPlayerNetId)
	{
		UE_LOG_CROSSPLAY_PREFERENCES(Warning, TEXT("Net ID of owning player is invalid, operation cancelled."))
		return;
	}

	// Retrieve current config
	Subsystem->RetrieveCrossplayPreference(
		OwningPlayerNetId,
		FOnRetrieveCrossplayPreferenceCompleted::CreateWeakLambda(this, [this](bool bSucceeded, bool bEnabled)
		{
			W_OptionEnabled->InitOption(TEXT_CROSSPLAY_CHECKBOX_NAME, bEnabled);
			Ws_Outer->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
		}));
}

void UCrossplayPreferenceWidget::NativeOnDeactivated()
{
	Btn_Back->OnClicked().RemoveAll(this);

	Super::NativeOnDeactivated();
}

void UCrossplayPreferenceWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 825.0f, 160.0f));
}

void UCrossplayPreferenceWidget::UpdateCrossplayPreference()
{
	Ws_Outer->LoadingMessage = TEXT_SAVING;
	Ws_Outer->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	Btn_Back->SetIsEnabled(false);

	const FUniqueNetIdPtr OwningPlayerNetId = AccelByteWarsUtility::GetUserId(GetOwningPlayer());
	if (!OwningPlayerNetId)
	{
		UE_LOG_CROSSPLAY_PREFERENCES(Warning, TEXT("Net ID of owning player is invalid, operation cancelled."))
		return;
	}

	Subsystem->UpdateCrossplayPreference(
		OwningPlayerNetId,
		W_OptionEnabled->GetToggleValue(),
		FOnUpdateCrossplayPreferenceCompleted::CreateWeakLambda(this, [this](bool bSucceeded)
		{
			DeactivateWidget();
			ResetUI();
		}));
}

void UCrossplayPreferenceWidget::ResetUI() const
{
	Ws_Outer->LoadingMessage = TEXT_LOADING;
	Ws_Outer->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	Btn_Back->SetIsEnabled(true);
}
