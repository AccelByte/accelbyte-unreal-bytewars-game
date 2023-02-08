// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/MainMenu/HelpOptions/Credits/CreditsWidget.h"
#include "ByteWarsCore/UI/MainMenu/HelpOptions/Credits/Components/CreditsRoleGroupWidget.h"
#include "ByteWarsCore/UI/MainMenu/HelpOptions/Credits/Components/CreditsEntry.h"
#include "Components/ScrollBox.h"

void UCreditsWidget::InitCredits()
{
	TMap<ECreditsRoleType, TWeakObjectPtr<UCreditsRoleGroupWidget>> RoleGroupMap;

	for (FCreditsData Credit : CreditsData)
	{
		// Initialize the role group if it is not yet.
		if (!RoleGroupMap.Contains(Credit.RoleType))
		{
			const TWeakObjectPtr<UCreditsRoleGroupWidget> NewRoleWidget = MakeWeakObjectPtr<UCreditsRoleGroupWidget>(CreateWidget<UCreditsRoleGroupWidget>(this, CreditsRoleGroupClass.Get()));
			NewRoleWidget->InitData(ConvertCreditsRoleToText(Credit.RoleType));

			RoleGroupMap.Add(Credit.RoleType, NewRoleWidget);
			Scb_CreditsList->AddChild(NewRoleWidget.Get());
		}

		// Assign the credit to respective role group.
		const TWeakObjectPtr<UCreditsEntry> NewCredit = MakeWeakObjectPtr<UCreditsEntry>(CreateWidget<UCreditsEntry>(this, CreditsEntryClass.Get()));
		NewCredit->InitData(Credit);

		RoleGroupMap[Credit.RoleType]->AddChild(NewCredit.Get());
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() { bIsCreditsListInitialized = true; }, AutoScrollDelay, false);
}

void UCreditsWidget::ScrollCreditsToEnd(const float DeltaTime, const float ScrollSpeed)
{
	if (!bIsCreditsListInitialized)
	{
		return;
	}

	const float CurrentOffset = Scb_CreditsList->GetScrollOffset();
	const float TargetOffset = Scb_CreditsList->GetScrollOffsetOfEnd();
	const float DesiredOffset = FMath::FInterpTo(CurrentOffset, TargetOffset, DeltaTime, ScrollSpeed);
	Scb_CreditsList->SetScrollOffset(DesiredOffset);
}

void UCreditsWidget::ResetCreditsList()
{
	Scb_CreditsList->ClearChildren();
	Scb_CreditsList->SetScrollOffset(0.0f);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	bIsCreditsListInitialized = false;
}

void UCreditsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	InitCredits();
}

void UCreditsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	ResetCreditsList();
}

void UCreditsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
	ScrollCreditsToEnd(InDeltaTime, AutoScrollSpeed);
}