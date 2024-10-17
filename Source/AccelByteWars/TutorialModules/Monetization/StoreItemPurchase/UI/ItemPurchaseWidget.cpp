// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ItemPurchaseWidget.h"

#include "Component/ItemPurchaseButton.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsSequentialSelectionWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Monetization/InGameStoreEssentials/UI/StoreItemDetailWidget.h"
#include "Monetization/StoreItemPurchase/StoreItemPurchaseSubsystem.h"
#include "Monetization/WalletEssentials/UI/WalletBalanceWidget.h"

void UItemPurchaseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	W_Parent = GetFirstOccurenceOuter<UStoreItemDetailWidget>();
	ensure(W_Parent);

	StoreItemDataObject = W_Parent->StoreItemDataObject;
	ensure(StoreItemDataObject);

	PurchaseSubsystem = GetGameInstance()->GetSubsystem<UStoreItemPurchaseSubsystem>();
	ensure(PurchaseSubsystem);

	NativePlatformPurchaseSubsystem = GetGameInstance()->GetSubsystem<UNativePlatformPurchaseSubsystem>();
	ensure(NativePlatformPurchaseSubsystem);

	// setup UI
	SetupPurchaseButtons(StoreItemDataObject->GetPrices());
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
	Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
	Tb_Error->SetVisibility(ESlateVisibility::Collapsed);
	Ss_Amount->SetSelectedIndex(0);
	Ss_Amount->OnSelectionChangedDelegate.AddUObject(this, &ThisClass::UpdatePrice);

	// show amount if consumable
	Ss_Amount->SetVisibility(StoreItemDataObject->GetIsConsumable() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// setup delegate
	PurchaseSubsystem->OnCheckoutCompleteDelegates.AddUObject(this, &ThisClass::OnPurchaseComplete);
	NativePlatformPurchaseSubsystem->OnSynchPurchaseCompleteDelegates.BindUObject(this, &ThisClass::OnSynchPurchaseComplete);

	// set focus
	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		W_PurchaseButtonsOuter->GetChildAt(0)->SetUserFocus(GetOwningPlayer());
	}

	FTUESetup();
}

void UItemPurchaseWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	PurchaseSubsystem->OnCheckoutCompleteDelegates.RemoveAll(this);
	Ss_Amount->OnSelectionChangedDelegate.RemoveAll(this);
	NativePlatformPurchaseSubsystem->OnSynchPurchaseCompleteDelegates.Unbind();
}

void UItemPurchaseWidget::OnClickPurchase(const int32 PriceIndex) const
{
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	const bool bIsNativePlatformValid = IOnlineSubsystem::GetByPlatform() != nullptr;
	
	if (bIsNativePlatformValid &&
		StoreItemDataObject->GetItemType() == ITEM_TYPE_COINS)
	{
		NativePlatformPurchaseSubsystem->OpenPlatformStore(
			GetOwningPlayer(), 
			StoreItemDataObject, 
			PriceIndex, 
			NativePlatformPurchaseSubsystem->GetItemMapping());

		return;
	}

	PurchaseSubsystem->CreateNewOrder(
		GetOwningPlayer(),
		StoreItemDataObject,
		PriceIndex,
		GetSelectedAmount());
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
		if (TWeakObjectPtr<UWalletBalanceWidget> Widget = W_Parent->GetBalanceWidget(); Widget.IsValid())
		{
			Widget->UpdateBalance(true);
		}
	}
	else
	{
		Tb_Error->SetText(FText::FromString(Error.ErrorRaw));

		Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
		Tb_Error->SetVisibility(ESlateVisibility::Visible);
	}
}

void UItemPurchaseWidget::OnSynchPurchaseComplete(bool bWasSuccessful, const FString& Error) const
{
	FOnlineError mError(bWasSuccessful);
	mError.ErrorRaw = Error;

	OnPurchaseComplete(mError);
}

#pragma region "FTUE"
void UItemPurchaseWidget::FTUESetup() const
{
	if (W_Parent)
	{
		if (FFTUEDialogueModel* FTUE = FFTUEDialogueModel::GetMetadataById("ftue_purchase_itemid", W_Parent->FTUEDialogues))
		{
			FTUE->Button1.URLArguments[2].Argument = StoreItemDataObject->GetStoreItemId();
		}
	}
}
#pragma endregion 

#pragma region "UI"
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
	for (int i = 0; i < Prices.Num(); ++i)
	{
		UItemPurchaseButton* Entry = CreateWidget<UItemPurchaseButton>(this, PurchaseButtonClass);
		ensure(Entry);

		Entry->SetPrice(Prices[i], GetSelectedAmount());
		Entry->OnClicked().AddUObject(this, &ThisClass::OnClickPurchase, i);
		W_PurchaseButtonsOuter->AddChild(Entry);
	}
}

void UItemPurchaseWidget::UpdatePrice(const int32 SelectedIndex)
{
	SetupPurchaseButtons(StoreItemDataObject->GetPrices());
}

int32 UItemPurchaseWidget::GetSelectedAmount() const
{
	const FString AmountString = Ss_Amount->GetSelected().ToString();
	return FCString::Atoi(*AmountString);
}
#pragma endregion 
