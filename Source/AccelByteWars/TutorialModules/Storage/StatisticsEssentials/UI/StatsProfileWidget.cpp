// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StatsProfileWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/DynamicEntryBox.h"
#include "Components/StatsProfileWidgetEntry.h"

#include "Storage/StatisticsEssentials/StatsEssentialsSubsystem.h"

void UStatsProfileWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StatsEssentialsSubsystem = GetGameInstance()->GetSubsystem<UStatsEssentialsSubsystem>();
	ensure(StatsEssentialsSubsystem);
}

// @@@SNIPSTART StatsProfileWidget.cpp-NativeOnActivated
void UStatsProfileWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	StatsDataEntryList.Empty();
	StatsDataEntryList.Add(GAMESTATS_GameModeSinglePlayer, Deb_SinglePlayerStats);
	StatsDataEntryList.Add(GAMESTATS_GameModeElimination, Deb_EliminationStats);
	StatsDataEntryList.Add(GAMESTATS_GameModeTeamDeathmatch, Deb_TeamDeathmatchStats);

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Ws_Loader->OnRetryClicked.AddUObject(this, &ThisClass::QueryLocalUserStats);

	QueryLocalUserStats();
}
// @@@SNIPEND

void UStatsProfileWidget::NativeOnDeactivated()
{
	StatsDataEntryList.Empty();
	Btn_Back->OnClicked().Clear();
	Ws_Loader->OnRetryClicked.Clear();

	Super::NativeOnDeactivated();
}

void UStatsProfileWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

// @@@SNIPSTART StatsProfileWidget.cpp-QueryLocalUserStats
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "34"]}
void UStatsProfileWidget::QueryLocalUserStats()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance) 
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to query local user stats. Game instance is invalid."));
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	TArray<FString> StatsCodes{};
	for (const TTuple<FName, UDynamicEntryBox*>& EntryList : StatsDataEntryList)
	{
		FGameStatsData StatsData{};
		if (GameInstance->GetGameStatsDataById(EntryList.Key, StatsData)) 
		{
			StatsCodes.Append(StatsData.GetStatsCodes());
		}
	}

	if (StatsCodes.IsEmpty())
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to query local user stats. No statistics code to query."));
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	const bool bStarted = StatsEssentialsSubsystem->QueryLocalUserStats(
		AccelByteWarsUtility::GetLocalUserNum(GetOwningPlayer()),
		StatsCodes,
		FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &ThisClass::OnQueryLocalUserStatsComplete));

	Ws_Loader->SetWidgetState(bStarted ? EAccelByteWarsWidgetSwitcherState::Loading : EAccelByteWarsWidgetSwitcherState::Error);
}
// @@@SNIPEND

// @@@SNIPSTART StatsProfileWidget.cpp-OnQueryLocalUserStatsComplete
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-4", "71"]}
void UStatsProfileWidget::OnQueryLocalUserStatsComplete(
	const FOnlineError& ResultState,
	const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
{
	if (!ResultState.bSucceeded)
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to handle on-complete query local user stats. Error: %s"), *ResultState.ErrorMessage.ToString());
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to handle on-complete query local user stats. Game instance is invalid."));
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	FUniqueNetIdPtr UserId = AccelByteWarsUtility::GetUserId(GetOwningPlayer());
	if (!UserId.IsValid())
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to handle on-complete query local user stats. User id is invalid."));
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	// Get stats for the target user.
	TSharedPtr<const FOnlineStatsUserStats> TargetUserStats = nullptr;
	for (TSharedRef<const FOnlineStatsUserStats> const& UserStats : UsersStatsResult)
	{
		if (UserStats.Get().Account.Get() == UserId.ToSharedRef().Get())
		{
			TargetUserStats = UserStats;
			break;
		}
	}
	if (!TargetUserStats) 
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to handle on-complete query local user stats. Local user's statistics is not found."));
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	// Generate stats entries.
	for (const TTuple<FName, UDynamicEntryBox*>& EntryList : StatsDataEntryList)
	{
		FGameStatsData StatsData{};
		TWeakObjectPtr<UDynamicEntryBox> TargetStatsList = EntryList.Value;
		if (!TargetStatsList.IsValid() || !GameInstance->GetGameStatsDataById(EntryList.Key, StatsData))
		{
			UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to handle on-complete query local user stats. Invalid properties to generate widget entry."));
			continue;
		}

		TargetStatsList->Reset(true);
		for (const FGameStatsModel& StatsModel : StatsData.GetStatsModels())
		{
			float StatsValue = 0;
			if (const FVariantData* Stats = TargetUserStats->Stats.Find(StatsModel.CodeName))
			{
				Stats->GetValue(StatsValue);
			}

			const TWeakObjectPtr<UStatsProfileWidgetEntry> WidgetEntry = TargetStatsList->CreateEntry<UStatsProfileWidgetEntry>();
			WidgetEntry->Setup(StatsModel.DisplayName, FText::AsNumber(StatsValue));
		}
	}

	Ws_Loader->SetWidgetState(UsersStatsResult.IsEmpty() ? EAccelByteWarsWidgetSwitcherState::Empty : EAccelByteWarsWidgetSwitcherState::Not_Empty);
}
// @@@SNIPEND
