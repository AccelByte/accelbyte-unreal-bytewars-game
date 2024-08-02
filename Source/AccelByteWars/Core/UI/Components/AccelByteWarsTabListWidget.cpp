// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsTabListWidget.h"

#include "AccelByteWarsButtonBase.h"
#include "CommonActionWidget.h"
#include "CommonInputSubsystem.h"
#include "Groups/CommonButtonGroupBase.h"

void UAccelByteWarsTabListWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	NextTabAction->SetInputAction(NextTabInputActionData);
	PreviousTabAction->SetInputAction(PreviousTabInputActionData);

	SetLinkedSwitcher(PreLinkedSwitcher);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	if (IsDesignTime() && ButtonPreviewNum > 0)
	{
		W_TabButtonContainer->ClearChildren();
		for (int i = 0; i < ButtonPreviewNum; ++i)
		{
			UAccelByteWarsButtonBase* Button = CreateWidget<UAccelByteWarsButtonBase>(this, PresetButtonClass);
			Button->SetButtonText(FText::FromString(TEXT("Test")));
			Button->SetIsFocusable(false);
			Button->SetPadding(PaddingBetweenButtons);
			if (IsValid(PresetButtonStyle))
			{
				Button->SetStyle(PresetButtonStyle);
			}
			W_TabButtonContainer->AddChild(Button);
		}
	}
#endif
}

void UAccelByteWarsTabListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// need to be called at least here, since some of the object that RegisterTab uses, constructed on NativeOnInitialized
	for (const FAccelByteWarsTabDescriptor& TabInfo : PreRegisteredTabInfos)
	{
		if (bUsePresetButtonClass)
		{
			RegisterTabWithPresets(TabInfo.TabId, TabInfo.ButtonText, TabInfo.TabContent, TabInfo.TabIndex);
		}
		else
		{
			RegisterTab(TabInfo.TabId, TabInfo.ButtonClass, TabInfo.TabContent, TabInfo.TabIndex);
		}
	}

	OnTabButtonCreation.AddDynamic(this, &ThisClass::HandleOnTabButtonCreation);
}

void UAccelByteWarsTabListWidget::NativeDestruct()
{
	Super::NativeDestruct();

	PreviouslySelectedTabId = FName();
	OnTabButtonCreation.RemoveAll(this);
}

void UAccelByteWarsTabListWidget::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabCreation_Implementation(TabNameID, TabButton);

	W_TabButtonContainer->AddChild(TabButton);
}

void UAccelByteWarsTabListWidget::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabRemoval_Implementation(TabNameID, TabButton);

	W_TabButtonContainer->RemoveChild(TabButton);
}

void UAccelByteWarsTabListWidget::ParentOnActivated()
{
	if (!PreviouslySelectedTabId.IsNone())
	{
		SelectTabByID(PreviouslySelectedTabId);
		PreviouslySelectedTabId = FName();
	}
}

void UAccelByteWarsTabListWidget::ParentOnDeactivated()
{
	PreviouslySelectedTabId = GetSelectedTabId();
}

bool UAccelByteWarsTabListWidget::RegisterTabWithPresets(
	const FName TabNameID,
	const FText ButtonText,
	UWidget* ContentWidget,
	const int32 TabIndex,
	const bool bForce)
{
	if (bForce && GetRegisteredTabsByID().Contains(TabNameID))
	{
		RemoveTab(TabNameID);
		TargetIndexMap.Remove(TabNameID);
	}

	// figure out what index should be used
	int32 CalculatedIndex = TabIndex;
	for (const TTuple<FName, int>& TargetIndex : TargetIndexMap)
	{
		if (TargetIndex.Value == TabIndex + 1)
		{
			if (GetRegisteredTabsByID().Contains(TargetIndex.Key))
			{
				CalculatedIndex = GetRegisteredTabsByID()[TargetIndex.Key].TabIndex;
			}
		}
	}

	const bool bResult = RegisterTab(TabNameID, PresetButtonClass, ContentWidget, CalculatedIndex);

	if (bResult)
	{
		TargetIndexMap.Add(TabNameID, TabIndex);

		UAccelByteWarsButtonBase* Button = Cast<UAccelByteWarsButtonBase>(GetTabButtonBaseByID(TabNameID));
		Button->SetButtonText(ButtonText);
		Button->SetIsFocusable(false);
		Button->SetPadding(PaddingBetweenButtons);
		if (IsValid(PresetButtonStyle))
		{
			Button->SetStyle(PresetButtonStyle);
		}
	}

	return bResult;
}

void UAccelByteWarsTabListWidget::HandleOnTabButtonCreation(FName TabId, UCommonButtonBase* TabButton)
{
	// check if linked switcher have the assigned widget
	if (const FCommonRegisteredTabInfo* TabInfo = GetRegisteredTabsByID().Find(TabId))
	{
		if (IsValid(TabInfo->ContentInstance) && LinkedSwitcher->GetChildIndex(TabInfo->ContentInstance) == INDEX_NONE)
		{
			// widget does not exist in switcher, add it
			LinkedSwitcher->AddChild(TabInfo->ContentInstance);
		}
	}
}
