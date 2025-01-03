// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsTabListWidget.h"

#include "AccelByteWarsButtonBase.h"
#include "AccelByteWarsWidgetSwitcher.h"
#include "CommonActionWidget.h"
#include "CommonInputSubsystem.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Blueprint/WidgetTree.h"

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
	OnTabSelected.AddDynamic(this, &ThisClass::HandleOnTabSelected);
}

void UAccelByteWarsTabListWidget::NativeDestruct()
{
	Super::NativeDestruct();

	PreviouslySelectedTabId = FName();
	OnTabButtonCreation.RemoveAll(this);
	OnTabSelected.RemoveAll(this);
}

void UAccelByteWarsTabListWidget::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	// Disable click sound for tab buttons.
	if (UAccelByteWarsButtonBase* Button = Cast<UAccelByteWarsButtonBase>(TabButton))
	{
		Button->SetAllowClickSound(false);
	}

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
	bool bIsRegistered = false;

	if (GetRegisteredTabsByID().Contains(TabNameID))
	{
		// If forced or tab button is invalid, remove the tab.
		UCommonButtonBase* TabButton = GetTabButtonBaseByID(TabNameID);
		if (bForce || !GetTabButtonBaseByID(TabNameID))
		{
			RemoveTab(TabNameID);
			TargetIndexMap.Remove(TabNameID);
		}
		// If tab button is valid, re-add to the tab list.
		else 
		{
			if (!W_TabButtonContainer->HasChild(TabButton)) 
			{
				HandleTabCreation_Implementation(TabNameID, TabButton);
			}
			bIsRegistered = true;
		}
	}
	
	if (!bIsRegistered) 
	{
		// Figure out what index should be used for the tab button.
		int32 CalculatedIndex = TabIndex;
		for (const TTuple<FName, int>& TargetIndex : TargetIndexMap)
		{
			if (TargetIndex.Value == TabIndex + 1 && GetRegisteredTabsByID().Contains(TargetIndex.Key))
			{
				CalculatedIndex = GetRegisteredTabsByID()[TargetIndex.Key].TabIndex;
			}
		}

		bIsRegistered = RegisterTab(TabNameID, PresetButtonClass, ContentWidget, CalculatedIndex);

		if (bIsRegistered)
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
	}

	return bIsRegistered;
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

void UAccelByteWarsTabListWidget::HandleOnTabSelected(FName TabId)
{
	const FCommonRegisteredTabInfo* TabInfo = GetRegisteredTabsByID().Find(TabId);
	if (!TabInfo) 
	{
		return;
	}

	/* Request to refresh content's widget switcher if any. 
	 * This is required to make sure the widget switcher state is in the correct state. */
	if (UUserWidget* ContentWidget = Cast<UUserWidget>(TabInfo->ContentInstance))
	{
		if (UAccelByteWarsWidgetSwitcher* Switcher = Cast<UAccelByteWarsWidgetSwitcher>(ContentWidget))
		{
			Switcher->ForceRefresh();
		}

		TArray<UWidget*> Children{};
		ContentWidget->WidgetTree->GetAllWidgets(Children);
		for (UWidget* Widget : Children)
		{
			if (UAccelByteWarsWidgetSwitcher* Switcher = Cast<UAccelByteWarsWidgetSwitcher>(Widget))
			{
				Switcher->ForceRefresh();
			}
		}
	}
}
