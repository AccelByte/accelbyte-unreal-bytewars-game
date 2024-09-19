// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem.h"
#include "ShopWidget.generated.h"

class UCommonButtonBase;
class UTileView;
class UWidgetSwitcher;
class UStoreItemListEntry;
class UPanelWidget;
class UStoreItemDetailWidget;
class UAccelByteWarsTabListWidget;
class UWalletBalanceWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UShopWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	void OnGetOrQueryCategoriesComplete(TArray<FOnlineStoreCategory> Categories);
	void OnRefreshCategoriesComplete(TArray<FOnlineStoreCategory> Categories);
	void OnGetOrQueryOffersComplete(const TArray<UStoreItemDataObject*> Offers) const;
	void OnStoreItemClicked(UObject* Item) const;

	void OnRefreshButtonClicked();

	UFUNCTION()
	void SwitchCategory(FName Id);

private:
	UPROPERTY(EditAnywhere)
	FString RootPath;

	UPROPERTY()
	UInGameStoreEssentialsSubsystem* StoreSubsystem;

#pragma region "UI"
public:
	inline static TMulticastDelegate<void(const APlayerController*)> OnActivatedMulticastDelegate;
	FSimpleMulticastDelegate OnRefreshButtonClickedDelegates;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchContent(EAccelByteWarsWidgetSwitcherState State) const;

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

	UWalletBalanceWidget* GetBalanceWidget() const; 
#pragma endregion 
};
