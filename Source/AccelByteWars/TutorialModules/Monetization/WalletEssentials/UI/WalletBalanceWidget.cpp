// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidget.h"

#include "Components/PanelWidget.h"
#include "Components/WalletBalanceWidgetEntry.h"
#include "Monetization/WalletEssentials/WalletEssentialsSubsystem.h"

// @@@SNIPSTART WalletBalanceWidget.cpp-NativeOnActivated
// @@@MULTISNIP WalletSubsystem {"selectedLines": ["1-2", "5-6", "16"]}
// @@@MULTISNIP Setup {"selectedLines": ["1-3", "10-16"], "highlightedLines": "{4-10}"}
void UWalletBalanceWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	WalletSubsystem = GetGameInstance()->GetSubsystem<UWalletEssentialsSubsystem>();
	ensure(WalletSubsystem);

	WalletSubsystem->OnQueryOrGetWalletInfoCompleteDelegates.AddUObject(this, &ThisClass::ShowBalance);

	// Update balance.
	W_Root->ClearChildren();
	CurrencyBalanceEntryMap.Empty();

	UpdateBalance(ECurrencyType::COIN);
	UpdateBalance(ECurrencyType::GEM);
}
// @@@SNIPEND

// @@@SNIPSTART WalletBalanceWidget.cpp-NativeOnDeactivated
void UWalletBalanceWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	WalletSubsystem->OnQueryOrGetWalletInfoCompleteDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART WalletBalanceWidget.cpp-ShowBalance
void UWalletBalanceWidget::ShowBalance(bool bWasSuccessful, const FAccelByteModelsWalletInfo& Response) const
{
	if (!bWasSuccessful)
	{
		// If failed, set all balance text to error.
		for (const TTuple<FString, UWalletBalanceWidgetEntry*>& CurrencyBalance : CurrencyBalanceEntryMap)
		{
			if (!CurrencyBalance.Value || CurrencyBalance.Key.IsEmpty())
			{
				continue;
			}

			CurrencyBalance.Value->Setup(TEXT_BALANCE_ERROR, FPreConfigCurrency::GetTypeFromCode(CurrencyBalance.Key));
		}
		return;
	}

	if (const UWalletBalanceWidgetEntry* Entry = *CurrencyBalanceEntryMap.Find(Response.CurrencyCode.ToUpper()))
	{
		Entry->Setup(
			FText::FromString(FString::FromInt(Response.Balance)),
			FPreConfigCurrency::GetTypeFromCode(Response.CurrencyCode));
	}
}
// @@@SNIPEND

// @@@SNIPSTART WalletBalanceWidget.cpp-UpdateBalance
// @@@MULTISNIP Setup {"selectedLines": ["1-10", "14"]}
void UWalletBalanceWidget::UpdateBalance(const ECurrencyType CurrencyType)
{
	const FString CurrencyCode = FPreConfigCurrency::GetCodeFromType(CurrencyType);

	UWalletBalanceWidgetEntry* Entry = CreateWidget<UWalletBalanceWidgetEntry>(this, CurrencyBalanceClass);
	ensure(Entry);

	Entry->Setup(FText::FromString("..."), CurrencyType);
	W_Root->AddChild(Entry);
	CurrencyBalanceEntryMap.Add(CurrencyCode, Entry);

	// Always retrieve from backend to make sure data is updated.
	WalletSubsystem->QueryOrGetWalletInfoByCurrencyCode(GetOwningPlayer(), CurrencyCode, true);
}
// @@@SNIPEND
