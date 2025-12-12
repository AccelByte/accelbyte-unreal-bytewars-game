// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "InventoryWidget.h"

#include "Core/UI/Components/AccelByteWarsTabListWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "Components/Image.h"
#include "CommonButtonBase.h"
#include "MediaPlayer.h"
#include "MediaSource.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Tl_Category->OnTabSelected.AddDynamic(this, &ThisClass::SwitchCategory);

	Tv_Content->SetSelectionMode(ESelectionMode::Single);
	Tv_Content->OnEntryWidgetGenerated().AddUObject(this, &ThisClass::OnItemEntryGenerated);
	Tv_Content->OnItemClicked().AddUObject(this, &ThisClass::OnItemListClicked);

	Btn_Equip->OnClicked().AddUObject(this, &ThisClass::OnEquipButtonClicked);
	Btn_Unequip->OnClicked().AddUObject(this, &ThisClass::OnUnequipButtonClicked);
}

void UInventoryWidget::NativeDestruct()
{
	CurrentCategory = nullptr;

	Tl_Category->OnTabSelected.Clear();

	Tv_Content->OnItemClicked().RemoveAll(this);
	Tv_Content->OnEntryWidgetGenerated().RemoveAll(this);
	Tv_Content->ClearListItems();

	Btn_Equip->OnClicked().Clear();
	Btn_Unequip->OnClicked().Clear();

	Img_Preview->SetBrush(DefaultPreviewBrush);
	if (IsValid(Mp_Preview))
	{
		Mp_Preview->Close();
	}

	Super::NativeDestruct();
}

void UInventoryWidget::SwitchCategory(const FName CategoryId)
{
	// If not provided, switch to the first category.
	if (CategoryId == NAME_None && !Categories.IsEmpty()) 
	{
		CurrentCategory = &Categories[0];
	}
	// If provided, switch to the specified category.
	else 
	{
		CurrentCategory = Categories.FindByPredicate([&](const FInventoryCategory& Category)
		{
			return Category.CategoryId == CategoryId;
		});
	}
	
	if (!CurrentCategory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to switch category. Category not found."));
		return;
	}

	Btn_Equip->SetVisibility(CurrentCategory->bShowEquipButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Btn_Unequip->SetVisibility(CurrentCategory->bShowUnequipButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	OnCategorySwitched.Broadcast(CurrentCategory);

	ScrollToEquippedItem();
	RefreshCategoryActions();
}

void UInventoryWidget::RefreshCategories()
{
	Tl_Category->RemoveAllTabs();
	Tv_Content->ClearListItems();

	if (Categories.IsEmpty()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to update item categories. No category found."));
		return;
	}

	// Refresh by reinitialize the category tabs.
	TSet<FName> CategoryIds;
	for (FInventoryCategory& Category : Categories)
	{
		Category.EquippedItem = nullptr;

		// Only register unique category.
		if (!CategoryIds.Contains(Category.CategoryId))
		{
			CategoryIds.Add(Category.CategoryId);
			Tl_Category->RegisterTabWithPresets(Category.CategoryId, Category.CategoryName);
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to register category tab. Category %s already exists."), *Category.CategoryId.ToString());
		}
	}
	CategoryIds.Empty();
}

void UInventoryWidget::RefreshCategoryActions()
{
	if (!CurrentCategory || !Tv_Content->GetSelectedItem())
	{
		Btn_Equip->SetIsEnabled(false);
		Btn_Unequip->SetIsEnabled(false);
		return;
	}

	const bool bIsSelectedEquipped = Tv_Content->GetSelectedItem() == CurrentCategory->EquippedItem;
	Btn_Equip->SetIsEnabled(!bIsSelectedEquipped);
	Btn_Unequip->SetIsEnabled(bIsSelectedEquipped);
}

void UInventoryWidget::SetPreview(const FSlateBrush Brush)
{
	Img_Preview->SetBrush(Brush);
	Ws_Preview->SetActiveWidget(Img_Preview);
}

void UInventoryWidget::SetPreview(UMediaSource* Media)
{
	if (Media) 
	{
		Mp_Preview->OpenSource(Media);
		Ws_Preview->SetActiveWidget(Img_MediaPreview);
	}
	else 
	{
		ClearPreview();
	}
}

void UInventoryWidget::ClearPreview()
{
	Ws_Preview->SetActiveWidget(Tb_NoPreview);
	Tb_Title->SetText(FText::GetEmpty());
	Tb_Description->SetText(FText::GetEmpty());
}

void UInventoryWidget::SetPreviewDetails(const FText& Title, const FText& Description)
{
	Tb_Title->SetText(Title);
	Tb_Description->SetText(Description);
}

void UInventoryWidget::ScrollToEquippedItem()
{
	int32 SelectedItemIndex = 0;
	if (CurrentCategory && CurrentCategory->EquippedItem) 
	{
		SelectedItemIndex = Tv_Content->GetIndexForItem(CurrentCategory->EquippedItem);
	}

	Tv_Content->SetSelectedIndex(SelectedItemIndex);
	Tv_Content->ScrollIndexIntoView(SelectedItemIndex);

	OnItemListClicked(Tv_Content->GetSelectedItem());
}

void UInventoryWidget::OnEquipButtonClicked()
{
	if (!CurrentCategory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to equip item. Category not found."));
		return;
	}

	UObject* PrevEquippedItem = CurrentCategory->EquippedItem;
	UObject* NewEquippedItem = Tv_Content->GetSelectedItem();

	// Toggle off highlight on previously equipped item.
	if (UAccelByteWarsWidgetEntry* PrevEquippedItemEntry =
		Cast<UAccelByteWarsWidgetEntry>(Tv_Content->GetEntryWidgetFromItem(PrevEquippedItem)))
	{
		PrevEquippedItemEntry->Execute_ToggleHighlight(PrevEquippedItemEntry, false);
	}

	// Toggle on highlight on newly equipped item.
	if (UAccelByteWarsWidgetEntry* EquippedItemEntry =
		Cast<UAccelByteWarsWidgetEntry>(Tv_Content->GetEntryWidgetFromItem(NewEquippedItem)))
	{
		EquippedItemEntry->Execute_ToggleHighlight(EquippedItemEntry, true);
	}

	CurrentCategory->EquippedItem = NewEquippedItem;
	RefreshCategoryActions();

	OnItemEquipClicked.Broadcast(CurrentCategory->EquippedItem, CurrentCategory);
}

void UInventoryWidget::OnUnequipButtonClicked()
{
	if (!CurrentCategory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to unequip item. Category not found."));
		return;
	}

	UObject* PrevEquippedItem = CurrentCategory->EquippedItem;
	CurrentCategory->EquippedItem = nullptr;
	RefreshCategoryActions();

	// Toggle off highlight on previously equipped item.
	if (UAccelByteWarsWidgetEntry* PrevEquippedItemEntry =
		Cast<UAccelByteWarsWidgetEntry>(Tv_Content->GetEntryWidgetFromItem(PrevEquippedItem)))
	{
		PrevEquippedItemEntry->Execute_ToggleHighlight(PrevEquippedItemEntry, false);
	}

	OnItemUnequipClicked.Broadcast(PrevEquippedItem, CurrentCategory);
}

void UInventoryWidget::OnItemListClicked(UObject* Item)
{
	if (CurrentCategory && Item)
	{
		OnItemClicked.Broadcast(Item, CurrentCategory);
		RefreshCategoryActions();
	}
}

void UInventoryWidget::OnItemEntryGenerated(UUserWidget& EntryWidget)
{
	// Highlight the equipped item.
	UAccelByteWarsWidgetEntry* ItemEntry = Cast<UAccelByteWarsWidgetEntry>(&EntryWidget);
	if (CurrentCategory && ItemEntry)
	{
		UObject* ItemObject = ItemEntry->GetListItem();
		if (CurrentCategory->EquippedItem && ItemObject == CurrentCategory->EquippedItem)
		{
			ItemEntry->Execute_ToggleHighlight(ItemEntry, true);
			ScrollToEquippedItem();
		}
	}
}