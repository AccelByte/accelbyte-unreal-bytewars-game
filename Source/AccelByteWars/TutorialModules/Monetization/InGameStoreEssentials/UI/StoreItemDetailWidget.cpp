// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemDetailWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

#pragma region "UI"
void UStoreItemDetailWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
}

void UStoreItemDetailWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
}

void UStoreItemDetailWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 750.0f, 160.0f));
}

UWidget* UStoreItemDetailWidget::NativeGetDesiredFocusTarget() const
{
	return W_PurchaseOuter->HasAnyChildren() ? W_PurchaseOuter->GetChildAt(0) : Btn_Back;
}
#pragma endregion 

void UStoreItemDetailWidget::Setup(const UStoreItemDataObject* Object)
{
	// Copy the object since we don't want the ShouldShowPrices to be set on the original data.
	StoreItemDataObject = NewObject<UStoreItemDataObject>();
	StoreItemDataObject->Setup(Object);
	StoreItemDataObject->SetShouldShowPrices(false);

	W_ItemDetail->Setup(StoreItemDataObject);
}

TWeakObjectPtr<UAccelByteWarsActivatableWidget> UStoreItemDetailWidget::GetBalanceWidget() const
{
	TWeakObjectPtr<UAccelByteWarsActivatableWidget> Widget = nullptr;

	if (W_WalletOuter->HasAnyChildren())
	{
		UAccelByteWarsActivatableWidget* WidgetRawPtr = Cast<UAccelByteWarsActivatableWidget>(W_WalletOuter->GetChildAt(0));
		if (WidgetRawPtr)
		{
			Widget = MakeWeakObjectPtr(WidgetRawPtr);
		}
	}

	return Widget;
}
