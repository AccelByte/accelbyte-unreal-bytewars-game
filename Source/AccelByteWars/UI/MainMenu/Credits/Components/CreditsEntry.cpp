// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UI/MainMenu/Credits/Components/CreditsEntry.h"
#include "CommonTextBlock.h"

void UCreditsEntry::InitData(const FCreditsData& CreditData)
{
	Txt_Name->SetText(CreditData.Name);
	Txt_AdditionalDesc->SetText(CreditData.AdditionalDescription);
}