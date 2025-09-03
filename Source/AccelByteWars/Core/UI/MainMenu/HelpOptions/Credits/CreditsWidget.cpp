// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Credits/CreditsWidget.h"

#include "Camera/CameraActor.h"
#include "Core/UI/MainMenu/HelpOptions/Credits/Components/CreditsRoleGroupWidget.h"
#include "Core/UI/MainMenu/HelpOptions/Credits/Components/CreditsEntry.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

void UCreditsWidget::InitCredits()
{
	bShouldTick = true;
	TMap<ECreditsRoleType, TWeakObjectPtr<UCreditsRoleGroupWidget>> RoleGroupMap;

	CreditsDataTable->ForeachRow<FCreditsData>(TEXT(""), [&](const FName& RowName, const FCreditsData& RowData)
	{
		// Initialize the role group if it is not yet.
		if (!RoleGroupMap.Contains(RowData.RoleType))
		{
			const TWeakObjectPtr<UCreditsRoleGroupWidget> NewRoleWidget = MakeWeakObjectPtr<UCreditsRoleGroupWidget>(CreateWidget<UCreditsRoleGroupWidget>(this, CreditsRoleGroupClass.Get()));
			NewRoleWidget->InitData(ConvertCreditsRoleToText(RowData.RoleType));

			RoleGroupMap.Add(RowData.RoleType, NewRoleWidget);
			Scb_CreditsList->AddChild(NewRoleWidget.Get());
		}

		// Assign the credit to respective role group.
		const TWeakObjectPtr<UCreditsEntry> NewCredit = MakeWeakObjectPtr<UCreditsEntry>(CreateWidget<UCreditsEntry>(this, CreditsEntryClass.Get()));
		NewCredit->InitData(RowData);

		RoleGroupMap[RowData.RoleType]->AddChild(NewCredit.Get());
	});

	BeginCreditsAutoScroll();
}

void UCreditsWidget::BeginCreditsAutoScroll()
{
	// Perform delay in Task Graph.
	Async(EAsyncExecution::TaskGraph, [this]
	{
		FPlatformProcess::Sleep(AutoScrollDelay);

		if (bIsCreditsWidgetActive)
		{
			// Run timer on game thread to ensure thread safety.
			AsyncTask(ENamedThreads::GameThread, [this]
			{
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UCreditsWidget::ScrollCreditsToEnd, 0.05f, true);
			});
		}
	});
}

void UCreditsWidget::ScrollCreditsToEnd() const
{
	const float CurrentOffset = Scb_CreditsList->GetScrollOffset();
	const float TargetOffset = Scb_CreditsList->GetScrollOffsetOfEnd();

	if (CurrentOffset < TargetOffset)
	{
		const float IncrementedOffset = CurrentOffset + AutoScrollOffset;
		Scb_CreditsList->SetScrollOffset(IncrementedOffset);
	}
}

void UCreditsWidget::ResetCreditsList()
{
	Scb_CreditsList->ClearChildren();
	Scb_CreditsList->SetScrollOffset(0.0f);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UCreditsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	InitCredits();
	bIsCreditsWidgetActive = true;
}

void UCreditsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	ResetCreditsList();
	bIsCreditsWidgetActive = false;
}

void UCreditsWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	if (!bShouldTick)
	{
		return;
	}

	Super::NativeTick(MyGeometry, InDeltaTime);

	const AActor* Camera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
	const FVector TargetLocation = FVector(60.0f, 600.0f, 160.0f);

	if (Camera->GetActorLocation() != TargetLocation)
	{
		MoveCameraToTargetLocation(InDeltaTime, TargetLocation);
	}
	else
	{
		bShouldTick = false;
	}
}