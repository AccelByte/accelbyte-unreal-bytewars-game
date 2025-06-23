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

	// Update balance.
	W_Root->ClearChildren();
	CurrencyBalanceEntryMap.Empty();

	UpdateBalance(ECurrencyType::COIN);
	UpdateBalance(ECurrencyType::GEM);
}

void UWalletBalanceWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	WalletSubsystem->OnQueryOrGetWalletInfoCompleteDelegates.RemoveAll(this);
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
