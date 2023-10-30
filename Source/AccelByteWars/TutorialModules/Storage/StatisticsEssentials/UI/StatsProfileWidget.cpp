// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsProfileWidget.h"

#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/DynamicEntryBox.h"
#include "Components/StatsProfileWidgetEntry.h"

#include "Storage/StatisticsEssentials/StatsEssentialsSubsystem.h"

void UStatsProfileWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Retry->OnClicked().AddUObject(this, &ThisClass::StartQueryLocalUserStats);

	StartQueryLocalUserStats();
}

void UStatsProfileWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EssentialsSubsystem = GetGameInstance()->GetSubsystem<UStatsEssentialsSubsystem>();
	ensure(EssentialsSubsystem);
}

void UStatsProfileWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().Clear();
	Btn_Retry->OnClicked().Clear();
}

void UStatsProfileWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UStatsProfileWidget::StartQueryLocalUserStats()
{
	const int32 LocalUserNum = GetOwningPlayer()->GetLocalPlayer()->GetControllerId();
	const bool bStarted = EssentialsSubsystem->QueryLocalUserStats(
		LocalUserNum,
		{
			UStatsEssentialsSubsystem::StatsCode_HighestElimination,
			UStatsEssentialsSubsystem::StatsCode_KillCount,
			UStatsEssentialsSubsystem::StatsCode_HighestSinglePlayer,
			UStatsEssentialsSubsystem::StatsCode_HighestTeamDeathMatch
		},
		FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &ThisClass::OnQueryLocalUserStatsComplete));

	// Show loading
	const bool bLoading = Ws_Outer->GetActiveWidget() == W_LoadingOuter;
	Ws_Outer->SetActiveWidget((bLoading || bStarted) ? W_LoadingOuter : W_FailedOuter);
}

void UStatsProfileWidget::OnQueryLocalUserStatsComplete(
	const FOnlineError& ResultState,
	const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
{
	// clear previous entries
	Deb_StatsList->Reset();

	if (!ResultState.bSucceeded)
	{
		Ws_Outer->SetActiveWidget(W_FailedOuter);
		return;
	}

	for (const TSharedRef<const FOnlineStatsUserStats>& UsersStats : UsersStatsResult)
	{
		for (const TTuple<FString, FVariantData>& Stat : UsersStats->Stats)
		{
			// by default, AB OSS store stats value as float
			float StatValue;
			Stat.Value.GetValue(StatValue);

			// Add entry object
			const UStatsProfileWidgetEntry* WidgetEntry = Deb_StatsList->CreateEntry<UStatsProfileWidgetEntry>();
			WidgetEntry->Setup(
				FText::FromStringTable("/Game/TutorialModules/Storage/StatisticsEssentials/String/ST_StatsKey.ST_StatsKey", Stat.Key),
				FText::AsNumber(StatValue));
		}
	}

	// show empty message if user doesn't have any stats, show list otherwise
	Ws_Outer->SetActiveWidget(UsersStatsResult.Num() > 0 ? Deb_StatsList : W_EmptyOuter);
}
