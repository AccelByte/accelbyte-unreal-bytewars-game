// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ItemPurchaseWidget.h"

#include "Component/ItemPurchaseWidgetEntry.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/InGameStoreEssentials/UI/StoreItemDetailWidget.h"
#include "TutorialModules/StoreItemPurchase/StoreItemPurchaseSubsystem.h"
#include "TutorialModules/WalletEssentials/UI/WalletBalanceWidget.h"

void UItemPurchaseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	W_Parent = GetFirstOccurenceOuter<UStoreItemDetailWidget>();
	ensure(W_Parent);

	StoreItemDataObject = W_Parent->StoreItemDataObject;
	ensure(StoreItemDataObject);

	PurchaseSubsystem = GetGameInstance()->GetSubsystem<UStoreItemPurchaseSubsystem>();
	ensure(PurchaseSubsystem);

	// setup UI
	SetupPurchaseButtons(StoreItemDataObject->Prices);
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
	Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
	Tb_Error->SetVisibility(ESlateVisibility::Collapsed);

	// setup delegate
	PurchaseSubsystem->OnCheckoutCompleteDelegates.AddUObject(this, &ThisClass::OnPurchaseComplete);
}

void UItemPurchaseWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	PurchaseSubsystem->OnCheckoutCompleteDelegates.RemoveAll(this);
}

void UItemPurchaseWidget::OnClickPurchase(const FString& CurrencyCode) const
{
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	PurchaseSubsystem->Checkout(GetOwningPlayer(), StoreItemDataObject->ItemData->Id, 1, StoreItemDataObject->ItemData->bConsumable);
}

void UItemPurchaseWidget::OnPurchaseComplete(const FOnlineError& Error) const
{
	OnPurchaseCompleteMulticastDelegate.Broadcast(GetOwningPlayer());
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	if (Error.bSucceeded)
	{
		Tb_Success->SetVisibility(ESlateVisibility::Visible);
		Tb_Error->SetVisibility(ESlateVisibility::Collapsed);

		// update wallet
		if (UWalletBalanceWidget* Widget = W_Parent->GetBalanceWidget())
		{
			Widget->UpdateBalance(true);
		}
	}
	else
	{
		Tb_Error->SetText(Error.ErrorMessage);

		Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
		Tb_Error->SetVisibility(ESlateVisibility::Visible);
	}
}

UWidget* UItemPurchaseWidget::NativeGetDesiredFocusTarget() const
{
	UWidget* FocusTarget = Super::NativeGetDesiredFocusTarget();

	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		FocusTarget = W_PurchaseButtonsOuter->GetChildAt(0);
	}

	return FocusTarget;
}

void UItemPurchaseWidget::SetupPurchaseButtons(TArray<UStoreItemPriceDataObject*> Prices)
{
	W_PurchaseButtonsOuter->ClearChildren();
	for (const UStoreItemPriceDataObject* Price : Prices)
	{
		UItemPurchaseWidgetEntry* Entry = CreateWidget<UItemPurchaseWidgetEntry>(this, PurchaseButtonClass);
		ensure(Entry);

		Entry->Setup(Price->CurrencyCode, FOnPurchaseClicked::CreateUObject(this, &ThisClass::OnClickPurchase));
		W_PurchaseButtonsOuter->AddChild(Entry);
	}

	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		W_PurchaseButtonsOuter->GetChildAt(0)->SetUserFocus(GetOwningPlayer());
	}
}
