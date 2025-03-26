// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ShopWidget.h"

#include "CommonButtonBase.h"
#include "StoreItemDetailWidget.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsTabListWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Monetization/WalletEssentials/UI/WalletBalanceWidget.h"

void UShopWidget::NativeOnActivated()
{
	// Widget will load the FTUE later after finished loading store items. 
	bLoadFTUEImmediately = false;
	
	Super::NativeOnActivated();

	StoreSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreEssentialsSubsystem>();
	ensure(StoreSubsystem);

	NativePlatformPurchaseSubsystem = GetGameInstance()->GetSubsystem<UNativePlatformPurchaseSubsystem>();

	// Event binding
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Refresh->OnClicked().AddUObject(this, &ThisClass::OnRefreshButtonClicked);
	Tl_ItemCategory->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);
	Tv_ContentOuter->OnItemClicked().AddUObject(this, &ThisClass::OnStoreItemClicked);

	// Reset UI
	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);

	StoreSubsystem->GetOrQueryCategoriesByRootPath(
		GetOwningPlayer(),
		RootPath,
		FOnGetOrQueryCategories::CreateUObject(this, &ThisClass::OnGetOrQueryCategoriesComplete));

	if (NativePlatformPurchaseSubsystem)
	{
		// Query the native platform item mapping if the native platform is valid.
		NativePlatformPurchaseSubsystem->QueryItemMapping(GetOwningPlayer());
	}

	OnActivatedMulticastDelegate.Broadcast(GetOwningPlayer());
}

void UShopWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Refresh->OnClicked().RemoveAll(this);
	Tv_ContentOuter->OnItemClicked().RemoveAll(this);
	Tl_ItemCategory->OnTabSelected.RemoveAll(this);
	Tv_ContentOuter->ClearListItems();
}

void UShopWidget::OnGetOrQueryCategoriesComplete(TArray<FOnlineStoreCategory> Categories)
{
	if (Categories.IsEmpty())
	{
		SwitchContent(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	Tl_ItemCategory->RemoveAllTabs();
	SwitchContent(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	// register "All" categories entry
	Tl_ItemCategory->RegisterTabWithPresets(FName(RootPath), FText::FromString("All")); // TODO: Localization

	// register tab list
	for (const FOnlineStoreCategory& Category : Categories)
	{
		// only register end of branch category
		if (Category.SubCategories.IsEmpty())
		{
			Tl_ItemCategory->RegisterTabWithPresets(FName(Category.Id), Category.Description);
		}
	}
}

void UShopWidget::OnRefreshCategoriesComplete(TArray<FOnlineStoreCategory> Categories)
{
	FName LastSelectedTab = Tl_ItemCategory->GetSelectedTabId();
	Tl_ItemCategory->SetVisibility(ESlateVisibility::Visible);
	
	//temporary unbind, as it will automatically select first tab when rebuild tab list, while we want to keep the previous tab selection 
	Tl_ItemCategory->OnTabSelected.RemoveDynamic(this, &ThisClass::SwitchCategory);
	
	OnGetOrQueryCategoriesComplete(Categories);

	//reselect tab if exist, otherwise select all item tab
	const bool bLastSelectedTabExist = Categories.ContainsByPredicate([&LastSelectedTab](FOnlineStoreCategory& Category)
	{
		return Category.SubCategories.IsEmpty() && Category.Id.Equals(LastSelectedTab.ToString());
	});
	const FName TabSelectionTarget = bLastSelectedTabExist ? LastSelectedTab : FName(RootPath);
	Tl_ItemCategory->SelectTabByID(TabSelectionTarget);
	StoreSubsystem->GetOrQueryOffersByCategory(
		GetOwningPlayer(),
		TabSelectionTarget.ToString(),
		FOnGetOrQueryOffersByCategory::CreateUObject(this, &ThisClass::OnGetOrQueryOffersComplete),
		true);

	//re-add after manually select and switch category, because re-add before it will be unstable	
	if(IsActivated())
	{
		Tl_ItemCategory->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);
	}
}

void UShopWidget::OnGetOrQueryOffersComplete(const TArray<UStoreItemDataObject*> Offers) const
{
	TArray<UStoreItemDataObject*> ExistingItems;
	for (UObject* Object : Tv_ContentOuter->GetListItems())
	{
		if (UStoreItemDataObject* ExistingItem = Cast<UStoreItemDataObject>(Object))
		{
			ExistingItems.Add(ExistingItem);
		}
	}
	ExistingItems.Append(Offers);
	Tv_ContentOuter->SetListItems(ExistingItems);

	SwitchContent(ExistingItems.IsEmpty() ?
		EAccelByteWarsWidgetSwitcherState::Empty : EAccelByteWarsWidgetSwitcherState::Not_Empty);
}

void UShopWidget::OnStoreItemClicked(UObject* Item) const
{
	if (!IsValid(DetailWidgetClass))
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUI = GameInstance->GetBaseUIWidget();
	if (!BaseUI)
	{
		return;
	}

	UAccelByteWarsActivatableWidget* Widget = BaseUI->PushWidgetToStack(EBaseUIStackType::Menu, DetailWidgetClass);

	UStoreItemDetailWidget* ItemDetailWidget = Cast<UStoreItemDetailWidget>(Widget);
	const UStoreItemDataObject* StoreItem = Cast<UStoreItemDataObject>(Item);
	if (!ItemDetailWidget || !StoreItem)
	{
		return;
	}

	ItemDetailWidget->Setup(StoreItem);
}

void UShopWidget::OnRefreshButtonClicked()
{
	OnRefreshButtonClickedDelegates.Broadcast();

	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);

	Tv_ContentOuter->ClearListItems();	
	Tl_ItemCategory->SetVisibility(ESlateVisibility::Hidden);

	//refresh store item
	StoreSubsystem->GetOrQueryCategoriesByRootPath(
		GetOwningPlayer(),
		RootPath,
		FOnGetOrQueryCategories::CreateUObject(this, &ThisClass::OnRefreshCategoriesComplete),
		true);
	
	//refresh balance
	UWalletBalanceWidget* BalanceWidget = GetBalanceWidget();
	if(BalanceWidget != nullptr)
	{
		BalanceWidget->UpdateBalance(true);
	}
}

void UShopWidget::SwitchCategory(FName Id)
{
	Tv_ContentOuter->ClearListItems();
	StoreSubsystem->GetOrQueryOffersByCategory(
		GetOwningPlayer(),
		Id.ToString(),
		FOnGetOrQueryOffersByCategory::CreateUObject(this, &ThisClass::OnGetOrQueryOffersComplete));
}

#pragma region "UI"
void UShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

UWidget* UShopWidget::NativeGetDesiredFocusTarget() const
{
	UWidget* FocusTarget = Btn_Back;
	if (!Tv_ContentOuter->GetListItems().IsEmpty())
	{
		FocusTarget = Tv_ContentOuter;
	}
	return FocusTarget;
}

void UShopWidget::SwitchContent(EAccelByteWarsWidgetSwitcherState State) const
{
	UWidget* FocusTarget;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		FocusTarget = Btn_Back;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		FocusTarget = Tv_ContentOuter;
		// Initialize FTUE after item shops available 
		InitializeFTUEDialogues(bOnActivatedInitializeFTUE);
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		FocusTarget = Btn_Back;
		break;
	default:
		FocusTarget = Btn_Back;
	}

	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_Loader->SetWidgetState(State);

	//disable refresh button on loading state
	Btn_Refresh->SetIsEnabled(State != EAccelByteWarsWidgetSwitcherState::Loading);
}

UWalletBalanceWidget* UShopWidget::GetBalanceWidget() const
{
	UWalletBalanceWidget* Widget = nullptr;

	if (W_WalletOuter->HasAnyChildren())
	{
		Widget = Cast<UWalletBalanceWidget>(W_WalletOuter->GetChildAt(0));
	}

	return Widget;
}
#pragma endregion 
