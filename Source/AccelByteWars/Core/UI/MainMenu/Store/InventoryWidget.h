// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "InventoryWidget.generated.h"

class UAccelByteWarsTabListWidget;
class UWidgetSwitcher;
class UCommonButtonBase;
class UUserWidget;
class UMediaPlayer;
class UMediaSource;
class UPanelWidget;
class UTextBlock;
class UTileView;
class UImage;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemClicked, UObject* /* Item */, FInventoryCategory* /* Category */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryEquipItemClicked, UObject* /* Item */, FInventoryCategory* /* Category */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryUnequipItemClicked, UObject* /* Item */, FInventoryCategory* /* Category */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryCategorySwitched, FInventoryCategory* /* Category */);

USTRUCT(BlueprintType)
struct FInventoryCategory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName CategoryId = NAME_None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FText CategoryName = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EItemType ItemType = EItemType::None;

	UPROPERTY(BlueprintReadOnly)
	UObject* EquippedItem = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bShowEquipButton = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bShowUnequipButton = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UInGameItemDataAsset* DefaultItemDataAsset = nullptr;
};

UCLASS()
class ACCELBYTEWARS_API UInventoryWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * @brief Switch to the specified category.
	 * @param CategoryId The category id to switch to. If not provided, switch to the first category.
	 */
	UFUNCTION()
	void SwitchCategory(const FName CategoryId = NAME_None);

	/**
	 * @brief Refresh the item categories and reinitialize the category tabs.
	 */
	void RefreshCategories();

	/**
	 * @brief Refresh category actions such as equip and unequip button states.
	 */
	void RefreshCategoryActions();

	/**
	 * @brief Get the current active category.
	 */
	const FInventoryCategory* GetCurrentActiveCategory() { return CurrentCategory; }

	/**
	 * @brief Set the list items to be displayed in the inventory.
	 */
	template <typename ItemObjectT, typename AllocatorType = FDefaultAllocator>
	void SetListItems(const TArray<ItemObjectT, AllocatorType>& InListItems)
	{
		Tv_Content->SetListItems(InListItems);

		if (InListItems.IsEmpty()) 
		{
			Ws_Content->SetActiveWidget(Tb_NoItems);
			ClearPreview();
		}
		else 
		{
			Ws_Content->SetActiveWidget(Tv_Content);
		}
	}
	
	/**
	 * @brief Set the preview image.
	 */
	void SetPreview(const FSlateBrush Brush);

	/**
	 * @brief Set the preview video.
	 */
	void SetPreview(UMediaSource* Media);

	/**
	 * @brief Rest the preview to empty.
	 */
	void ClearPreview();

	/**
	 * @brief Set the preview details such as title and description.
	 */
	void SetPreviewDetails(const FText& Title, const FText& Description);

	void ScrollToEquippedItem();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FInventoryCategory> Categories;

	FOnInventoryCategorySwitched OnCategorySwitched;
	FOnInventoryItemClicked OnItemClicked;
	FOnInventoryEquipItemClicked OnItemEquipClicked;
	FOnInventoryUnequipItemClicked OnItemUnequipClicked;

protected:
	UFUNCTION()
	void OnEquipButtonClicked();

	UFUNCTION()
	void OnUnequipButtonClicked();

	UFUNCTION()
	void OnItemListClicked(UObject* Item);
	
	void OnItemEntryGenerated(UUserWidget& EntryWidget);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Preview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Content;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Preview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_MediaPreview;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMediaPlayer* Mp_Preview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_NoPreview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Title;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_NoItems;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_Content;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsTabListWidget* Tl_Category;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Equip;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Unequip;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FSlateBrush DefaultPreviewBrush;

	FInventoryCategory* CurrentCategory;
};
