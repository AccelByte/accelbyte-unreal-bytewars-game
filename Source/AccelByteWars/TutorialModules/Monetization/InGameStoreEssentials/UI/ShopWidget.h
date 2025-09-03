// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem.h"
#include "Monetization/NativePlatformPurchase/NativePlatformPurchaseSubsystem.h"
#include "ShopWidget.generated.h"

class UCommonButtonBase;
class UTileView;
class UWidgetSwitcher;
class UStoreItemListEntry;
class UPanelWidget;
class UStoreItemDetailWidget;
class UAccelByteWarsTabListWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UShopWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART ShopWidget.h-protected
// @@@MULTISNIP QueryComplete {"selectedLines": ["1-4"]}
// @@@MULTISNIP OnClicked {"selectedLines": ["1", "6-10"]}
protected:
	void OnGetOrQueryCategoriesComplete(TArray<FOnlineStoreCategory> Categories);
	void OnRefreshCategoriesComplete(TArray<FOnlineStoreCategory> Categories);
	void OnGetOrQueryOffersComplete(const TArray<UStoreItemDataObject*> Offers) const;

	void OnStoreItemClicked(UObject* Item) const;
	void OnRefreshButtonClicked();

	UFUNCTION()
	void SwitchCategory(FName Id);
// @@@SNIPEND
	
// @@@SNIPSTART ShopWidget.h-private
// @@@MULTISNIP RootPath {"selectedLines": ["1-3"]}
// @@@MULTISNIP Subsystem {"selectedLines": ["1", "5-6"]}
private:
	UPROPERTY(EditAnywhere)
	FString RootPath;

	UPROPERTY()
	UInGameStoreEssentialsSubsystem* StoreSubsystem;
// @@@SNIPEND

#pragma region "UI"
// @@@SNIPSTART ShopWidget.h-public-delegates
public:
	inline static TMulticastDelegate<void(const APlayerController*)> OnActivatedMulticastDelegate;
	FSimpleMulticastDelegate OnRefreshButtonClickedDelegates;
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.h-protected-UI
// @@@MULTISNIP SwitchContent {"selectedLines": ["1", "5"]}
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchContent(EAccelByteWarsWidgetSwitcherState State) const;
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.h-private-UI
// @@@MULTISNIP Components {"selectedLines": ["1-18"]}
// @@@MULTISNIP DetailWidgetClass {"selectedLines": ["1", "20-21"]}
// @@@MULTISNIP Wallet {"selectedLines": ["1", "23-26"]}
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_ListOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Loader;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsTabListWidget* Tl_ItemCategory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Refresh;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> DetailWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_WalletOuter;

	UAccelByteWarsActivatableWidget* GetBalanceWidget() const; 
#pragma endregion
// @@@SNIPEND
};

