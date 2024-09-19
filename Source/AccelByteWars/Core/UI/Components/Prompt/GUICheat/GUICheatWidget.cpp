// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "GUICheatWidget.h"

#include "GUICheatModels.h"
#include "Components/ListView.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

void UGUICheatWidget::AddEntry(UGUICheatWidgetEntry* Entry)
{
	// Add entry
	CurrentGUICheatWidgetEntries.Add(Entry);

	UpdateUI();
}

void UGUICheatWidget::RemoveEntry(UGUICheatWidgetEntry* Entry)
{
	if (CurrentGUICheatWidgetEntries.Remove(Entry) > 0)
	{
		UpdateUI();
	}
}

void UGUICheatWidget::RemoveEntries(UTutorialModuleDataAsset* TutorialModuleDataAsset)
{
	bool bFound = false;
	CurrentGUICheatWidgetEntries.RemoveAll([TutorialModuleDataAsset, &bFound](const UGUICheatWidgetEntry* Searched)
	{
		const bool bToRemove = Searched->OwnerTutorialModule == TutorialModuleDataAsset;
		if (bToRemove)
		{
			bFound = true;
		}
		return bToRemove;
	});

	// If not found, skip process
	if (!bFound)
	{
		return;
	}

	UpdateUI();
}

bool UGUICheatWidget::IsEntriesEmpty() const
{
	return CurrentGUICheatWidgetEntries.IsEmpty();
}

void UGUICheatWidget::UpdateUI()
{
	Lv_EntriesOuter->SetListItems(CurrentGUICheatWidgetEntries);
}
