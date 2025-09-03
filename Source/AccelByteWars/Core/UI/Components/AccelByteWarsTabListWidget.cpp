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

DEFINE_LOG_CATEGORY(LogAccelByteWarsTabListWidget);

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

	NextTabAction->SetInputAction(NextTabInputActionData);
	PreviousTabAction->SetInputAction(PreviousTabInputActionData);

	SetLinkedSwitcher(LinkedSwitcher.Get());

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
	// Select the previous selected tab if any, otherwise select the first tab.
	if (!PreviouslySelectedTabId.IsNone())
	{
		SelectTabByID(PreviouslySelectedTabId);
		PreviouslySelectedTabId = FName();
	}
	else if (!PreRegisteredTabInfos.IsEmpty())
	{
		SelectTabByID(PreRegisteredTabInfos[0].TabId);
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
	TMap<FName, FCommonRegisteredTabInfo> TempTabs = GetRegisteredTabsByID();

	// If forced or the tab button is invalid, unregister the existing tab first.
	const UCommonButtonBase* TabButton = GetTabButtonBaseByID(TabNameID);
	if (bForce || !TabButton)
	{
		RemoveTab(TabNameID);
	}

	// If the target tab index is greater than current number of tabs, then append at the end.
	if (TabIndex <= INDEX_NONE || TabIndex >= TempTabs.Num())
	{
		bIsRegistered = RegisterTab(TabNameID, PresetButtonClass, ContentWidget, TempTabs.Num());
		if (bIsRegistered)
		{
			UpdateTabButtonStyle(TabNameID, ButtonText);
		}
	}
	// Otherwise, rebuild all tabs and insert new one at correct position
	else
	{
		RemoveAllTabs();

		// Sort existing tabs by tab index
		TArray<TPair<FName, FCommonRegisteredTabInfo>> SortedTabs = TempTabs.Array();
		SortedTabs.Sort([](const TPair<FName, FCommonRegisteredTabInfo>& A, const TPair<FName, FCommonRegisteredTabInfo>& B)
		{
			return A.Value.TabIndex < B.Value.TabIndex;
		});

		int32 CurrentIndex = 0;
		bool bInsertedNewTab = false;
		for (const TPair<FName, FCommonRegisteredTabInfo>& Tab : SortedTabs)
		{
			// Insert new tab at the desired index
			if (!bInsertedNewTab && Tab.Value.TabIndex == TabIndex)
			{
				bInsertedNewTab = true;
				bIsRegistered = RegisterTab(TabNameID, PresetButtonClass, ContentWidget, CurrentIndex++);
				if (bIsRegistered)
				{
					UpdateTabButtonStyle(TabNameID, ButtonText);
				}
			}

			/* If the existing tabs is not properly rebuilt, then treat it as fatal operation.
			 * Then, remove all tabs and notify that the target index is invalid. */
			if (!RegisterTab(Tab.Key, Tab.Value.TabButtonClass, Tab.Value.ContentInstance, CurrentIndex++))
			{
				UE_LOG_ACCELBYTEWARSTABLISTWIDGET(
					Warning, 
					TEXT("Failed to insert new tab with ID %s and rebuild existing tabs. Target index %d is invalid."),
					*TabNameID.ToString(), TabIndex);
				RemoveAllTabs();
				return false;
			}
			// Otherwise, update the registered existing tab button style.
			else 
			{
				UpdateTabButtonStyle(Tab.Key);
			}
		}
	}
	
	return bIsRegistered;
}

bool UAccelByteWarsTabListWidget::UpdateTabButtonStyle(const FName& TabNameID, const FText& ButtonText)
{
	if (UAccelByteWarsButtonBase* Button = Cast<UAccelByteWarsButtonBase>(GetTabButtonBaseByID(TabNameID)))
	{
		/* Make sure the button tab has a label.
		 * If the text param is empty, fallback to existing button text.
		 * Otherwise fallback to the tab ID. */
		Button->SetButtonText(
			ButtonText.IsEmpty() ?
			Button->GetButtonText().IsEmpty() ?
			FText::FromString(TabNameID.ToString()) :
			Button->GetButtonText() :
			ButtonText);

		Button->SetIsFocusable(false);
		Button->SetPadding(PaddingBetweenButtons);

		if (IsValid(PresetButtonStyle))
		{
			Button->SetStyle(PresetButtonStyle);
		}

		return true;
	}

	return false;
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
	PreviouslySelectedTabId = TabId;

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
