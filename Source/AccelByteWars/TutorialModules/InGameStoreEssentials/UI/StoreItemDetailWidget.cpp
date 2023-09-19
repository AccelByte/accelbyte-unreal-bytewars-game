// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemDetailWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/MainMenu/Store/StoreItemListEntry.h"
#include "TutorialModules/WalletEssentials/UI/WalletBalanceWidget.h"

void UStoreItemDetailWidget::Setup(UStoreItemDataObject* Object)
{
	StoreItemDataObject = Object;
	W_ItemDetail->Setup(Object);
}

void UStoreItemDetailWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
}

UWalletBalanceWidget* UStoreItemDetailWidget::GetBalanceWidget() const
{
	UWalletBalanceWidget* Widget = nullptr;

	if (W_WalletOuter->HasAnyChildren())
	{
		Widget = Cast<UWalletBalanceWidget>(W_WalletOuter->GetChildAt(0));
	}

	return Widget;
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
