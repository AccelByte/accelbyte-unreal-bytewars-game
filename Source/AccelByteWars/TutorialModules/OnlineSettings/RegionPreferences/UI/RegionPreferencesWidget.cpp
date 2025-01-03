// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegionPreferencesWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#define WARNING_TEXT_SHOW_TIME 5.0f
#define WARNING_TEXT_CYCLE_TIME 0.5f

void URegionPreferencesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
}

void URegionPreferencesWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	Btn_Refresh->OnClicked().AddUObject(this, &ThisClass::OnRefreshButtonClicked);
	Lv_Regions->ClearListItems();
	
	RegionPreferencesSubsystem->OnWarningMinimumRegionCount.AddUObject(this, &ThisClass::ShowWarningText);
	RegionPreferencesSubsystem->OnRefreshRegionComplete.AddUObject(this, &ThisClass::OnRefreshRegionComplete);
	
	SetupUI();
}

void URegionPreferencesWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Refresh->OnClicked().Clear();
	Lv_Regions->ClearListItems();

	RegionPreferencesSubsystem->OnWarningMinimumRegionCount.RemoveAll(this);
	RegionPreferencesSubsystem->OnRefreshRegionComplete.RemoveAll(this);
	
	WarningTextRemainingTime = 0;
}

void URegionPreferencesWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 825.0f, 160.0f));
	UpdateWarningText(InDeltaTime);
}

void URegionPreferencesWidget::ShowWarningText()
{
	Text_Warning->SetVisibility(ESlateVisibility::Visible);
	WarningTextRemainingTime = WARNING_TEXT_SHOW_TIME;
}

void URegionPreferencesWidget::UpdateWarningText(float DeltaTime)
{
	// Warning text will be blinking for 5 seconds
	if(WarningTextRemainingTime > 0)
	{
		const float Fractional = FMath::Frac(WarningTextRemainingTime);
		Text_Warning->SetVisibility(Fractional < WARNING_TEXT_CYCLE_TIME ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		WarningTextRemainingTime -= DeltaTime;
	}
	else if(Text_Warning->GetVisibility() == ESlateVisibility::Visible)
	{
		WarningTextRemainingTime = 0;
		Text_Warning->SetVisibility(ESlateVisibility::Hidden);
	}
}

void URegionPreferencesWidget::SetupUI()
{	
	if(RegionPreferencesSubsystem->GetRegionInfos().Num() > 0)
	{
		Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
		Lv_Regions->SetListItems(RegionPreferencesSubsystem->GetRegionInfos());
	}
	else
	{
		Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
		RegionPreferencesSubsystem->RefreshRegionLatency();
	}

	Text_Warning->SetText(WARNING_MINIMUM_REGION);
	Text_Warning->SetVisibility(ESlateVisibility::Hidden);
}

void URegionPreferencesWidget::OnRefreshButtonClicked()
{
	Btn_Refresh->SetIsEnabled(false);
	
	Lv_Regions->ClearListItems();
	Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	
	RegionPreferencesSubsystem->RefreshRegionLatency();
}

void URegionPreferencesWidget::OnRefreshRegionComplete(bool bWasSucceed)
{
	Btn_Refresh->SetIsEnabled(true);

	if(bWasSucceed)
	{
		if(RegionPreferencesSubsystem->GetRegionInfos().Num() > 0)
		{
			Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
			SetupUI();
		}
		else
		{
			Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
		}
	}
	else
	{
		Ws_Regions->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}
