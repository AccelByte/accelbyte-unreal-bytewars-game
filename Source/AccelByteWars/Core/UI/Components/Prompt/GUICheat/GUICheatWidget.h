// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "GUICheatWidget.generated.h"

class UGUICheatWidgetEntry;
class UListView;
class UTutorialModuleDataAsset;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UGUICheatWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void AddEntry(UGUICheatWidgetEntry* Entry);
	void RemoveEntry(UGUICheatWidgetEntry* Entry);
	void RemoveEntries(UTutorialModuleDataAsset* TutorialModuleDataAsset);
	bool IsEntriesEmpty() const;

private:
	UPROPERTY()
	TArray<UGUICheatWidgetEntry*> CurrentGUICheatWidgetEntries;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_EntriesOuter;

	void UpdateUI();
};
