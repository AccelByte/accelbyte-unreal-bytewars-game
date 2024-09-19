// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "IngameRegionInfo.h"
#include "Components/TextBlock.h"
#include "Core/System/AccelByteWarsGameInstance.h"

void UIngameRegionInfo::NativeConstruct()
{
	Super::NativeConstruct();
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
}

void UIngameRegionInfo::NativeOnActivated()
{
	Super::NativeOnActivated();
	if(RegionPreferencesSubsystem != nullptr)
	{
		if(RegionPreferencesSubsystem->ShouldShowLatencyInGame())
		{
			FString CurrentRegion = RegionPreferencesSubsystem->GetCurrentGameSessionRegion();
			Text_Region->SetText(FText::Format(INGAME_REGION_TEXT_FMT, FText::FromString(CurrentRegion)));
			Text_Latency->SetText(FText::Format(INGAME_LATENCY_TEXT_FMT, FText::FromString(TEXT("---"))));
			RegionPreferencesSubsystem->OnLatencyRefreshComplete.AddUObject(this, &ThisClass::OnLatencyRefreshed);
			RegionPreferencesSubsystem->StartLatencyRefreshTimer();
			SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UIngameRegionInfo::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if(RegionPreferencesSubsystem != nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		RegionPreferencesSubsystem->StopLatencyRefreshTimer();
		RegionPreferencesSubsystem->OnLatencyRefreshComplete.RemoveAll(this);
	}
}

void UIngameRegionInfo::OnLatencyRefreshed(float Latency)
{
	constexpr float LowerLimit = 0.0;
	constexpr float Tolerance = 1e-3;
	
	const FText LatencyValueText = std::fabs(Latency - LowerLimit) > Tolerance ? FText::Format(LATENCY_TEXT_FMT, Latency) : FText::FromString(TEXT("---"));
	Text_Latency->SetText(FText::Format(INGAME_LATENCY_TEXT_FMT, LatencyValueText));
}
