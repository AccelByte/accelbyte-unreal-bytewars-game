// Fill out your copyright notice in the Description page of Project Settings.


#include "TutorialModules/Module-4/UI/StatsProfileWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/DynamicEntryBox.h"
#include "Components/StatsProfileWidgetEntry.h"

#include "TutorialModules/Module-4/StatsEssentialsSubsystem_Starter.h"

void UStatsProfileWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
}

void UStatsProfileWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	EssentialsSubsystem = GetGameInstance()->GetSubsystem<UStatsEssentialsSubsystem_Starter>();
	ensure(EssentialsSubsystem);
}

void UStatsProfileWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().Clear();
	Btn_Retry->OnClicked().Clear();
}

void UStatsProfileWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}
