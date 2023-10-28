// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ItemPurchaseWidgetEntry.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

void UItemPurchaseWidgetEntry::Setup(const FString& InCurrencyCode, const FOnPurchaseClicked& OnClicked)
{
	CurrencyCode = InCurrencyCode.ToUpper();
	const FString Text = FString::Printf(TEXT("%s %s"), *TEXT_PURCHASE.ToString(), *CurrencyCode);

	Btn_Purchase->SetButtonText(FText::FromString(Text));
	Btn_Purchase->OnClicked().AddWeakLambda(this, [OnClicked, this]()
	{
		OnClicked.ExecuteIfBound(CurrencyCode);
	});
}
