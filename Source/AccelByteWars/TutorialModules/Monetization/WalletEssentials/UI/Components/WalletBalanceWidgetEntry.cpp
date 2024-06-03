// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidgetEntry.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#if WITH_EDITOR
void UWalletBalanceWidgetEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		I_CurrencySymbol->SetBrush(DebugCurrencyType == ECurrencyType::COIN ? Brush_Coin : Brush_Gem);
	}
}
#endif

void UWalletBalanceWidgetEntry::Setup(const FText& Balance, const ECurrencyType CurrencyType) const
{
	Tb_Balance->SetText(Balance);
	I_CurrencySymbol->SetBrush(CurrencyType == ECurrencyType::COIN ? Brush_Coin : Brush_Gem);
}
