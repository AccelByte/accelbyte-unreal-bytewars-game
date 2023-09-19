// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidgetEntry.h"

#include "Components/TextBlock.h"

void UWalletBalanceWidgetEntry::Setup(const FText& Balance, const FText& CurrencyCode) const
{
	Tb_Balance->SetText(Balance);
	Tb_CurrencyCode->SetText(CurrencyCode);
}
