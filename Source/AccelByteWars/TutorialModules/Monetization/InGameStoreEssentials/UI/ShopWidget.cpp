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

// @@@SNIPSTART ShopWidget.cpp-NativeOnActivated
// @@@MULTISNIP Subsystem {"selectedLines": ["1-2", "8-9", "26"]}
// @@@MULTISNIP Setup {"selectedLines": ["1-2", "11-15", "17-18", "25-26"], "highlightedLines": "{6-11}"}
// @@@MULTISNIP Combine {"selectedLines": ["1-2", "17-23", "25-26"], "highlightedLines": "{7-10}"}
void UShopWidget::NativeOnActivated()
{
	// Widget will load the FTUE later after finished loading store items. 
	bLoadFTUEImmediately = false;
	
	Super::NativeOnActivated();

	StoreSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreEssentialsSubsystem>();
	ensure(StoreSubsystem);

	// Event binding.
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Refresh->OnClicked().AddUObject(this, &ThisClass::OnRefreshButtonClicked);
	Tl_ItemCategory->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);
	Tv_ContentOuter->OnItemClicked().AddUObject(this, &ThisClass::OnStoreItemClicked);
	Tv_ContentOuter->ClearListItems();

	// Reset UI.
	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);

	StoreSubsystem->GetOrQueryCategoriesByRootPath(
		GetOwningPlayer(),
		RootPath,
		FOnGetOrQueryCategories::CreateUObject(this, &ThisClass::OnGetOrQueryCategoriesComplete));

	OnActivatedMulticastDelegate.Broadcast(GetOwningPlayer());
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP Cleanup {"highlightedLines": "{6-9}"}
void UShopWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Refresh->OnClicked().RemoveAll(this);
	Tv_ContentOuter->OnItemClicked().RemoveAll(this);
	Tl_ItemCategory->OnTabSelected.RemoveAll(this);
	Tv_ContentOuter->ClearListItems();
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-OnGetOrQueryCategoriesComplete
void UShopWidget::OnGetOrQueryCategoriesComplete(TArray<FOnlineStoreCategory> Categories)
{
	if (Categories.IsEmpty())
	{
		SwitchContent(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	Tl_ItemCategory->RemoveAllTabs();
	SwitchContent(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	// Register "All" categories entry.
	Tl_ItemCategory->RegisterTabWithPresets(FName(RootPath), FText::FromString("All")); // TODO: Localization

	// Register tab list.
	for (const FOnlineStoreCategory& Category : Categories)
	{
		// Only register end of branch category.
		if (Category.SubCategories.IsEmpty())
		{
			Tl_ItemCategory->RegisterTabWithPresets(FName(Category.Id), Category.Description);
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-OnRefreshCategoriesComplete
// @@@MULTISNIP Setup {"selectedLines": ["1-16", "24-29"]}
void UShopWidget::OnRefreshCategoriesComplete(TArray<FOnlineStoreCategory> Categories)
{
	FName LastSelectedTab = Tl_ItemCategory->GetSelectedTabId();
	Tl_ItemCategory->SetVisibility(ESlateVisibility::Visible);
	
	// Temporary unbind, as it will automatically select first tab when rebuild tab list, while we want to keep the previous tab selection. 
	Tl_ItemCategory->OnTabSelected.RemoveDynamic(this, &ThisClass::SwitchCategory);
	
	OnGetOrQueryCategoriesComplete(Categories);

	// Reselect tab if exist, otherwise select all item tab.
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

	// Re-add after manually select and switch category, because re-add before it will be unstable.	
	if(IsActivated())
	{
		Tl_ItemCategory->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);
	}
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-OnGetOrQueryOffersComplete
void UShopWidget::OnGetOrQueryOffersComplete(const TArray<UStoreItemDataObject*> Offers) const
{
	Tv_ContentOuter->SetListItems(Offers);
	SwitchContent(Tv_ContentOuter->GetNumItems() <= 0 ?
		EAccelByteWarsWidgetSwitcherState::Empty : EAccelByteWarsWidgetSwitcherState::Not_Empty);
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-OnStoreItemClicked
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
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-OnRefreshButtonClicked
// @@@MULTISNIP Setup {"selectedLines": ["1-8", "17-24"]}
void UShopWidget::OnRefreshButtonClicked()
{
	OnRefreshButtonClickedDelegates.Broadcast();

	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);

	Tv_ContentOuter->ClearListItems();	
	Tl_ItemCategory->SetVisibility(ESlateVisibility::Hidden);

	// Refresh store item.
	StoreSubsystem->GetOrQueryCategoriesByRootPath(
		GetOwningPlayer(),
		RootPath,
		FOnGetOrQueryCategories::CreateUObject(this, &ThisClass::OnRefreshCategoriesComplete),
		true);
	
	// Refresh balance.
	UAccelByteWarsActivatableWidget* BalanceWidget = GetBalanceWidget();
	if(BalanceWidget != nullptr)
	{
		BalanceWidget->DeactivateWidget();
		BalanceWidget->ActivateWidget();
	}
}
// @@@SNIPEND

// @@@SNIPSTART ShopWidget.cpp-SwitchCategory
// @@@MULTISNIP Setup {"selectedLines": ["1-3", "8"]}
void UShopWidget::SwitchCategory(FName Id)
{
	Tv_ContentOuter->ClearListItems();
	StoreSubsystem->GetOrQueryOffersByCategory(
		GetOwningPlayer(),
		Id.ToString(),
		FOnGetOrQueryOffersByCategory::CreateUObject(this, &ThisClass::OnGetOrQueryOffersComplete));
}
// @@@SNIPEND

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

// @@@SNIPSTART ShopWidget.cpp-SwitchContent
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
		// Initialize FTUE after item shops available.
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

	// Disable refresh button on loading state.
	Btn_Refresh->SetIsEnabled(State != EAccelByteWarsWidgetSwitcherState::Loading);
}
// @@@SNIPEND

UAccelByteWarsActivatableWidget* UShopWidget::GetBalanceWidget() const
{
	UAccelByteWarsActivatableWidget* Widget = nullptr;

	if (W_WalletOuter->HasAnyChildren())
	{
		Widget = Cast<UAccelByteWarsActivatableWidget>(W_WalletOuter->GetChildAt(0));
	}

	return Widget;
}
#pragma endregion
