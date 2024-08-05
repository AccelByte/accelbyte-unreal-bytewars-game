// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletBalanceWidget.h"

#include "Components/PanelWidget.h"
#include "Components/WalletBalanceWidgetEntry.h"
#include "Monetization/WalletEssentials/WalletEssentialsSubsystem.h"

void UWalletBalanceWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	WalletSubsystem = GetGameInstance()->GetSubsystem<UWalletEssentialsSubsystem>();
	ensure(WalletSubsystem);

	WalletSubsystem->OnQueryOrGetWalletInfoCompleteDelegates.AddUObject(this, &ThisClass::ShowBalance);

	UpdateBalance(false);
}

void UWalletBalanceWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	WalletSubsystem->OnQueryOrGetWalletInfoCompleteDelegates.RemoveAll(this);
}

void UWalletBalanceWidget::UpdateBalance(const bool bAlwaysRequestToService)
{
	W_Root->ClearChildren();
	CurrencyBalanceEntryMap.Empty();

	UpdateBalanceInternal(ECurrencyType::COIN, bAlwaysRequestToService);
	UpdateBalanceInternal(ECurrencyType::GEM, bAlwaysRequestToService);
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
			FPreConfigCurrency::GetTypeFromCode(Response.CurrencyCode));
	}
}

void UWalletBalanceWidget::UpdateBalanceInternal(const ECurrencyType CurrencyType, const bool bAlwaysRequestToService)
{
	const FString CurrencyCode = FPreConfigCurrency::GetCodeFromType(CurrencyType);

	UWalletBalanceWidgetEntry* Entry = CreateWidget<UWalletBalanceWidgetEntry>(this, CurrencyBalanceClass);
	ensure(Entry);

	Entry->Setup(FText::FromString("..."), CurrencyType);
	W_Root->AddChild(Entry);
	CurrencyBalanceEntryMap.Add(CurrencyCode, Entry);

	WalletSubsystem->QueryOrGetWalletInfoByCurrencyCode(GetOwningPlayer(), CurrencyCode, bAlwaysRequestToService);
}
