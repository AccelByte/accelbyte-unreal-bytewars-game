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

void UShopWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	StoreSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreEssentialsSubsystem>();
	ensure(StoreSubsystem);

	// event binding
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Tl_ItemCategory->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);
	Tv_ContentOuter->OnItemClicked().AddUObject(this, &ThisClass::OnStoreItemClicked);

	// reset UI
	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);

	StoreSubsystem->GetOrQueryCategoriesByRootPath(
		GetOwningPlayer(),
		RootPath,
		FOnGetOrQueryCategories::CreateUObject(this, &ThisClass::OnGetOrQueryCategoriesComplete));

	OnActivatedMulticastDelegate.Broadcast(GetOwningPlayer());
}

void UShopWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	Tv_ContentOuter->OnItemClicked().RemoveAll(this);
	Tl_ItemCategory->OnTabSelected.RemoveAll(this);
	Tv_ContentOuter->ClearListItems();

	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UShopWidget::OnGetOrQueryCategoriesComplete(TArray<FOnlineStoreCategory> Categories)
{
	if (Categories.IsEmpty())
	{
		SwitchContent(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	Tl_ItemCategory->RemoveAllTabs();

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
	// play sound
	FSlateApplication::Get().PlaySound(PressedSlateSound);

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
	UStoreItemDataObject* StoreItem = Cast<UStoreItemDataObject>(Item);
	if (!ItemDetailWidget || !StoreItem)
	{
		return;
	}

	ItemDetailWidget->Setup(StoreItem);
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
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		FocusTarget = Btn_Back;
		break;
	default:
		FocusTarget = Btn_Back;
	}

	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_Loader->SetWidgetState(State);
}
#pragma endregion 
