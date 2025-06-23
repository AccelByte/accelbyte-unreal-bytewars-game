// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ItemPurchaseWidget_Starter.h"

#include "Component/ItemPurchaseButton.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsSequentialSelectionWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Monetization/InGameStoreEssentials/UI/StoreItemDetailWidget.h"
#include "Monetization/StoreItemPurchase/StoreItemPurchaseSubsystem_Starter.h"

void UItemPurchaseWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	W_Parent = GetFirstOccurenceOuter<UStoreItemDetailWidget>();
	ensure(W_Parent);

	StoreItemDataObject = W_Parent->StoreItemDataObject;
	ensure(StoreItemDataObject.IsValid());

	PurchaseSubsystem = GetGameInstance()->GetSubsystem<UStoreItemPurchaseSubsystem_Starter>();
	ensure(PurchaseSubsystem);

#pragma region "Tutorial"
	// put your code here
#pragma endregion

	// set focus
	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		W_PurchaseButtonsOuter->GetChildAt(0)->SetUserFocus(GetOwningPlayer());
	}

	FTUESetup();
}

void UItemPurchaseWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}

#pragma region "Tutorial"
// put your code here
#pragma endregion

#pragma region "FTUE"
void UItemPurchaseWidget_Starter::FTUESetup() const
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
UWidget* UItemPurchaseWidget_Starter::NativeGetDesiredFocusTarget() const
{
	UWidget* FocusTarget = Super::NativeGetDesiredFocusTarget();

	if (W_PurchaseButtonsOuter->HasAnyChildren())
	{
		FocusTarget = W_PurchaseButtonsOuter->GetChildAt(0);
	}

	return FocusTarget;
}

void UItemPurchaseWidget_Starter::SetupPurchaseButtons(TArray<UStoreItemPriceDataObject*> Prices)
{
	W_PurchaseButtonsOuter->ClearChildren();
	for (int i = 0; i < Prices.Num(); ++i)
	{
		UItemPurchaseButton* Entry = CreateWidget<UItemPurchaseButton>(this, PurchaseButtonClass);
		ensure(Entry);

#pragma region "Tutorial"
		// put your code here
#pragma endregion 
		W_PurchaseButtonsOuter->AddChild(Entry);
	}
}

void UItemPurchaseWidget_Starter::UpdatePrice(const int32 SelectedIndex)
{
	SetupPurchaseButtons(StoreItemDataObject->GetPrices());
}

int32 UItemPurchaseWidget_Starter::GetSelectedAmount() const
{
	const FString AmountString = Ss_Amount->GetSelected().ToString();
	return FCString::Atoi(*AmountString);
}
#pragma endregion 
