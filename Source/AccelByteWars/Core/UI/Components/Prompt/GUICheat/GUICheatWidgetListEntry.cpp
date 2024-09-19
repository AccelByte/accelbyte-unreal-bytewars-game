// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "GUICheatWidgetListEntry.h"

#include "CommonButtonBase.h"
#include "GUICheatParamListEntry.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

void UGUICheatWidgetListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	UGUICheatWidgetEntry* Data = Cast<UGUICheatWidgetEntry>(ListItemObject);
	if (!Data)
	{
		return;
	}

	// Setup UI
	Tb_Name->SetText(Data->Name);
	Btn_Execute->OnClicked().AddUObject(this, &ThisClass::OnExecuteClicked, Data->OnClicked);

	// Setup param UI
	ParamData.Empty();
	for (const FText& ParamName : Data->ParamNames)
	{
		UGUICheatParamListEntryData* ParamListEntryData = NewObject<UGUICheatParamListEntryData>();
		ParamListEntryData->ParamName = ParamName;
		ParamData.Add(ParamListEntryData);
	}
	Lv_ParamOuter->SetListItems(ParamData);
}

void UGUICheatWidgetListEntry::OnExecuteClicked(FOnGUICheatWidgetEntryClicked OnClicked) const
{
	// Construct param
	TArray<FString> Params;
	for (const UUserWidget* Widget : Lv_ParamOuter->GetDisplayedEntryWidgets())
	{
		const UGUICheatParamListEntry* ParamEntry = Cast<UGUICheatParamListEntry>(Widget);
		if (!ParamEntry)
		{
			continue;
		}

		const FString Param = ParamEntry->GetParamValue();
		Params.Add(Param);
	}

	OnClicked.Broadcast(Params);
}
