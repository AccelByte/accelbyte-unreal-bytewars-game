// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidget.h"

#include "Components/PanelWidget.h"
#include "Components/WalletBalanceWidgetEntry.h"
#include "TutorialModules/WalletEssentials/WalletEssentialsSubsystem.h"

void UWalletBalanceWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	WalletSubsystem = GetGameInstance()->GetSubsystem<UWalletEssentialsSubsystem>();
	ensure(WalletSubsystem);

	WalletSubsystem->OnGetWalletInfoCompleteDelegates.AddUObject(this, &ThisClass::ShowBalance);

	UpdateBalance(false);
}

void UWalletBalanceWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	WalletSubsystem->OnGetWalletInfoCompleteDelegates.RemoveAll(this);
}

void UWalletBalanceWidget::UpdateBalance(const bool bAlwaysRequestToService)
{
	W_Root->ClearChildren();
	CurrencyBalanceEntryMap.Empty();

	for (const FString& Currency : Currencies)
	{
		const FString CurrencyUpper = Currency.ToUpper();
		UWalletBalanceWidgetEntry* Entry = CreateWidget<UWalletBalanceWidgetEntry>(this, CurrencyBalanceClass);
		ensure(Entry);

		Entry->Setup(FText::FromString("..."), FText::FromString(CurrencyUpper));
		W_Root->AddChild(Entry);
		CurrencyBalanceEntryMap.Add(CurrencyUpper, Entry);

		WalletSubsystem->GetWalletInfoByCurrencyCode(GetOwningPlayer(), CurrencyUpper, bAlwaysRequestToService);
	}
}

void UWalletBalanceWidget::ShowBalance(bool bWasSuccessful, const FAccelByteModelsWalletInfo& Response) const
{
	if (!bWasSuccessful)
	{
		return;
	}

	if (const UWalletBalanceWidgetEntry* Entry = *CurrencyBalanceEntryMap.Find(Response.CurrencyCode.ToUpper()))
	{
		Entry->Setup(
			bWasSuccessful ? FText::FromString(FString::FromInt(Response.Balance)) : TEXT_BALANCE_ERROR,
			FText::FromString(Response.CurrencyCode.ToUpper()));
	}
}
