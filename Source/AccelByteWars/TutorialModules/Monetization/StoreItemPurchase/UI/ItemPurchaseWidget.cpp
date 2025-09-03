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

// @@@SNIPSTART ItemPurchaseWidget.cpp-NativeOnActivated
// @@@MULTISNIP StoreItemDataObject {"selectedLines": ["1-2", "5-9", "50"]}
// @@@MULTISNIP PurchaseSubsystem {"selectedLines": ["1-2", "11-12", "50"]}
// @@@MULTISNIP Setup {"selectedLines": ["1-12", "16-25", "43-50"]}
// @@@MULTISNIP Finishing {"selectedLines": ["1-12", "16-36", "43-50"]}
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

	// Setup UI
	SetupPurchaseButtons(StoreItemDataObject->GetPrices());
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
	Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
	Tb_Error->SetVisibility(ESlateVisibility::Collapsed);
	Ss_Amount->SetSelectedIndex(0);
	Ss_Amount->OnSelectionChangedDelegate.AddUObject(this, &ThisClass::UpdatePrice);

	// Show amount if consumable
	Ss_Amount->SetVisibility(StoreItemDataObject->GetIsConsumable() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	// Hide amount if native item
	for (UStoreItemPriceDataObject* Price : StoreItemDataObject->GetPrices()) {
		if (Price->GetCurrencyType() == ECurrencyType::NATIVE) {
			Ss_Amount->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	}

	// Setup delegate
	PurchaseSubsystem->OnCheckoutCompleteDelegates.AddUObject(this, &ThisClass::OnPurchaseComplete);
	if (NativePlatformPurchaseSubsystem) 
	{
		// Sync the native platform purchases if the native platform is valid.
		NativePlatformPurchaseSubsystem->OnSyncPurchaseCompleteDelegates.BindUObject(this, &ThisClass::OnSyncPurchaseComplete);
	}

	// Set focus
	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		W_PurchaseButtonsOuter->GetChildAt(0)->SetUserFocus(GetOwningPlayer());
	}

	FTUESetup();
}
// @@@SNIPEND

// @@@SNIPSTART ItemPurchaseWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP Setup {"selectedLines": ["1-3", "6", "12"]}
// @@@MULTISNIP Finishing {"selectedLines": ["1-6", "12"]}
void UItemPurchaseWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	PurchaseSubsystem->OnCheckoutCompleteDelegates.RemoveAll(this);
	Ss_Amount->OnSelectionChangedDelegate.RemoveAll(this);

	if (NativePlatformPurchaseSubsystem) 
	{
		NativePlatformPurchaseSubsystem->OnSyncPurchaseCompleteDelegates.Unbind();
	}
}
// @@@SNIPEND

// @@@SNIPSTART ItemPurchaseWidget.cpp-OnClickPurchase
// @@@MULTISNIP Setup {"selectedLines": ["1-3", "31"]}
// @@@MULTISNIP Finishing {"selectedLines": ["1-3", "25-31"]}
void UItemPurchaseWidget::OnClickPurchase(const int32 PriceIndex) const
{
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	bool bIsNativeItem = false;
	TArray<EItemSkuPlatform> SkuPlatforms;
	StoreItemDataObject->GetSkuMap().GenerateKeyArray(SkuPlatforms);
	for(const EItemSkuPlatform& SkuPlatform : SkuPlatforms)
	{
		if(SkuPlatform != EItemSkuPlatform::AccelByte)
		{
			bIsNativeItem = true;
			break;
		}
	}

	// Open the native platform store if the native platform is valid and the item type is supported.
	if (NativePlatformPurchaseSubsystem &&
		NativePlatformPurchaseSubsystem->IsNativePlatformSupported() &&
		bIsNativeItem)
	{
		NativePlatformPurchaseSubsystem->CheckoutItem(GetOwningPlayer(), StoreItemDataObject);
		return;
	}

	// Otherwise, fallback to use AccelByte platform to purchase the item.
	PurchaseSubsystem->CreateNewOrder(
		GetOwningPlayer(),
		StoreItemDataObject,
		PriceIndex,
		GetSelectedAmount());
}
// @@@SNIPEND

// @@@SNIPSTART ItemPurchaseWidget.cpp-OnPurchaseComplete
void UItemPurchaseWidget::OnPurchaseComplete(const FOnlineError& Error) const
{
	OnPurchaseCompleteMulticastDelegate.Broadcast(GetOwningPlayer());
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	if (Error.bSucceeded)
	{
		Tb_Success->SetVisibility(ESlateVisibility::Visible);
		Tb_Error->SetVisibility(ESlateVisibility::Collapsed);

		// update wallet
		if (TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = W_Parent->GetBalanceWidget(); Widget.IsValid())
		{
			Widget->DeactivateWidget();
			Widget->ActivateWidget();
		}
	}
	else
	{
		Tb_Error->SetText(FText::FromString(Error.ErrorRaw));

		Tb_Success->SetVisibility(ESlateVisibility::Collapsed);
		Tb_Error->SetVisibility(ESlateVisibility::Visible);
	}
}
// @@@SNIPEND

void UItemPurchaseWidget::OnSyncPurchaseComplete(bool bWasSuccessful, const FString& Error) const
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

// @@@SNIPSTART ItemPurchaseWidget.cpp-SetupPurchaseButtons
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
// @@@SNIPEND

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
