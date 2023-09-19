// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "StoreItemDetailWidget.generated.h"

class UCommonButtonBase;
class UStoreItemListEntry;
class UPanelWidget;
class UWalletBalanceWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStoreItemDetailWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UStoreItemDataObject* StoreItemDataObject;

	void Setup(UStoreItemDataObject* Object);

#pragma region "UI related"
public:
	virtual void NativeOnActivated() override;
	UWalletBalanceWidget* GetBalanceWidget() const;

protected:
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UStoreItemListEntry* W_ItemDetail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_PurchaseOuter;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_WalletOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

#pragma endregion 
};
