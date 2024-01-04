// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsSequentialSelectionWidget.h"

#include "CommonButtonBase.h"
#include "CommonInputSubsystem.h"
#include "Components/TextBlock.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

void UAccelByteWarsSequentialSelectionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	Tb_DisplayName->SetText(DisplayName);
	SetSelection(Selections, CurrentIndex);
}

void UAccelByteWarsSequentialSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_CycleRight->OnClicked().AddUObject(this, &ThisClass::CycleSelection, ECycleDirection::RIGHT);
	Btn_CycleLeft->OnClicked().AddUObject(this, &ThisClass::CycleSelection, ECycleDirection::LEFT);
}

void UAccelByteWarsSequentialSelectionWidget::NativeDestruct()
{
	Btn_CycleRight->OnClicked().RemoveAll(this);
	Btn_CycleLeft->OnClicked().RemoveAll(this);

	Super::NativeDestruct();
}

FNavigationReply UAccelByteWarsSequentialSelectionWidget::NativeOnNavigation(
	const FGeometry& MyGeometry,
	const FNavigationEvent& InNavigationEvent,
	const FNavigationReply& InDefaultReply)
{
	if (InNavigationEvent.GetNavigationType() == EUINavigation::Left)
	{
		CycleSelection(ECycleDirection::LEFT);
		return FNavigationReply::Explicit(nullptr);
	}
	else if (InNavigationEvent.GetNavigationType() == EUINavigation::Right)
	{
		CycleSelection(ECycleDirection::RIGHT);
		return FNavigationReply::Explicit(nullptr);
	}

	return Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);
}

void UAccelByteWarsSequentialSelectionWidget::SetSelection(TArray<FText>& InSelections, int32 DefaultIndex)
{
	Selections = InSelections;
	CurrentIndex = DefaultIndex < InSelections.Num() && DefaultIndex >= 0 ? DefaultIndex : INDEX_NONE;

	DrawSelection();
}

void UAccelByteWarsSequentialSelectionWidget::ClearSelection()
{
	Selections.Empty();
	CurrentIndex = INDEX_NONE;

	DrawSelection();
}

void UAccelByteWarsSequentialSelectionWidget::SetDisplayName(const FText InText) const
{
	Tb_DisplayName->SetText(InText);
}

void UAccelByteWarsSequentialSelectionWidget::SetSelectedIndex(const int32 Index)
{
	CurrentIndex = Index;
	DrawSelection();
}

void UAccelByteWarsSequentialSelectionWidget::CycleSelection(const ECycleDirection Direction)
{
	int32 Modifier = 0;
	switch (Direction)
	{
	case ECycleDirection::RIGHT:
		Modifier = 1;
		break;
	case ECycleDirection::LEFT:
		Modifier = -1;
		break;
	}
	CurrentIndex = AccelByteWarsUtility::PositiveModulo(CurrentIndex + Modifier, Selections.Num());

	DrawSelection();
}

void UAccelByteWarsSequentialSelectionWidget::DrawSelection()
{
	Tb_Selected->SetText(CurrentIndex > INDEX_NONE ? Selections[CurrentIndex] : FText());

	const bool bShouldEnableButton = Selections.Num() > 1;
	Btn_CycleRight->SetIsEnabled(bShouldEnableButton);
	Btn_CycleLeft->SetIsEnabled(bShouldEnableButton);
}
