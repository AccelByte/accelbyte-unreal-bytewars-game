// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Monetization/NativePlatformPurchase/NativePlatformPurchaseSubsystem.h"
#include "ItemPurchaseWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UTextBlock;
class UStoreItemPurchaseSubsystem;
class UStoreItemPriceDataObject;
class UStoreItemDetailWidget;
class UStoreItemDataObject;
class UItemPurchaseButton;
class UAccelByteWarsSequentialSelectionWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UItemPurchaseWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART ItemPurchaseWidget.h-public
public:
	inline static TMulticastDelegate<void(const APlayerController*)> OnPurchaseCompleteMulticastDelegate;
// @@@SNIPEND

// @@@SNIPSTART ItemPurchaseWidget.h-private
// @@@MULTISNIP PurchaseSubsystem {"selectedLines": ["1-3"]}
// @@@MULTISNIP StoreItemDataObject {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP Setup {"selectedLines": ["1", "11-12"]}
private:
	UPROPERTY()
	UStoreItemPurchaseSubsystem* PurchaseSubsystem;

	UPROPERTY()
	UStoreItemDataObject* StoreItemDataObject;

	UPROPERTY()
	UNativePlatformPurchaseSubsystem* NativePlatformPurchaseSubsystem;

	void OnClickPurchase(const int32 PriceIndex) const;
	void OnPurchaseComplete(const FOnlineError& Error) const;
	void OnSyncPurchaseComplete(bool bWasSuccessful,const FString& Error) const;
// @@@SNIPEND

#pragma region "FTUE"
private:
	void FTUESetup() const;
#pragma endregion 

#pragma region "UI"
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

// @@@SNIPSTART ItemPurchaseWidget.h-private-ui
// @@@MULTISNIP SetupPurchaseButtons {"selectedLines": ["1-2"]}
// @@@MULTISNIP UpdatePrice {"selectedLines": ["1", "3"]}
// @@@MULTISNIP GetSelectedAmount {"selectedLines": ["1", "4"]}
// @@@MULTISNIP UI {"selectedLines": ["1", "6-22"]}
private:
	void SetupPurchaseButtons(TArray<UStoreItemPriceDataObject*> Prices);
	void UpdatePrice(const int32 SelectedIndex);
	int32 GetSelectedAmount() const;

	UPROPERTY()
	UStoreItemDetailWidget* W_Parent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UItemPurchaseButton> PurchaseButtonClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_PurchaseButtonsOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Success;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Error;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsSequentialSelectionWidget* Ss_Amount;
#pragma endregion
// @@@SNIPEND
};
