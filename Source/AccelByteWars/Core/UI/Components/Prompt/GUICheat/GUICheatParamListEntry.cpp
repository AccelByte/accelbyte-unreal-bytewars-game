// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "GUICheatParamListEntry.h"

#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void UGUICheatParamListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	const UGUICheatParamListEntryData* Data = Cast<UGUICheatParamListEntryData>(ListItemObject);
	if (!Data)
	{
		return;
	}

	// Reset UI
	Tb_ParamName->SetText(Data->ParamName);
	Et_ParamValue->SetText(FText::GetEmpty());
}

FString UGUICheatParamListEntry::GetParamValue() const
{
	return Et_ParamValue->GetText().ToString();
}
