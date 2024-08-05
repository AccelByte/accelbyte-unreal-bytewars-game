// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InventoryWidget.h"
#include "Components/TileView.h"
#include "CommonButtonBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Components/MultiLineEditableText.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"

void UInventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	// bind
	Tv_Content->OnItemClicked().AddUObject(this, &UInventoryWidget::OnClickListItem);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Equip->OnClicked().AddUObject(this, &ThisClass::OnClickEquip);
	Btn_Unequip->OnClicked().AddUObject(this, &ThisClass::OnClickUnEquip);

	// reset UI
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	EntitlementsSubsystem->GetOrQueryUserEntitlements(
		GetOwningPlayer(),
		FOnGetOrQueryUserEntitlementsComplete::CreateUObject(this, &ThisClass::ShowEntitlements));

	SelectedItem = nullptr;
	SwitchToDefaultState();

	Btn_Equip->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Tv_Content->OnItemClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Equip->OnClicked().RemoveAll(this);
	Btn_Unequip->OnClicked().RemoveAll(this);
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	if (IsValid(MediaPlayer))
	{
		MediaPlayer->Close();
	}
	OnInventoryMenuDeactivated.Broadcast();
}

void UInventoryWidget::ShowEntitlements(const FOnlineError& Error, const TArray<UStoreItemDataObject*> Entitlements) const
{
	if (Error.bSucceeded)
	{
		// filter items
		TArray<UStoreItemDataObject*> FilteredEntitlements = Entitlements;
		FilteredEntitlements.RemoveAll([this](const UStoreItemDataObject* Item)
		{
			return bIsConsumable != Item->GetIsConsumable() || (Item->GetIsConsumable() && Item->GetCount() <= 0);
		});

		Tv_Content->ClearListItems();
		Tv_Content->SetListItems(FilteredEntitlements);

		Ws_Root->SetWidgetState(Tv_Content->GetListItems().IsEmpty()
			? EAccelByteWarsWidgetSwitcherState::Empty
			: EAccelByteWarsWidgetSwitcherState::Not_Empty);
		Tv_Content->SetUserFocus(GetOwningPlayer());
	}
	else
	{
		Ws_Root->ErrorMessage = Error.ErrorMessage;
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}

#pragma region "Byte Wars specific"
void UInventoryWidget::OnClickListItem(UObject* Object)
{
	if (UStoreItemDataObject* Item = Cast<UStoreItemDataObject>(Object))
	{
		SelectedItem = Item;
		if (!SelectedItem->GetSkuMap().Find(EItemSkuPlatform::AccelByte))
		{
			return;
		}

		// Get in-game item
		if (const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAssetBySku(
			EItemSkuPlatform::AccelByte,
			SelectedItem->GetSkuMap()[EItemSkuPlatform::AccelByte]))
		{
			/**
			 * bIsConsumable is used to differentiate power up and skin item in Byte Wars
			 * Skin will have a video preview, and power up will have an image preview
			 */
			if (bIsConsumable)
			{
				MediaPlayer->OpenSource(ItemDataAsset->PreviewVideo);
			}
			else
			{
				W_SelectedItemPreview->SetBrush(ItemDataAsset->PreviewImage);
			}
			Tb_Description->SetText(
				ItemDataAsset->Description.IsEmpty() ? FText::FromString(TEXT("")) : ItemDataAsset->Description);
		}
		else
		{
			SwitchToDefaultState();
		}

		Tb_SelectedItemTitle->SetText(SelectedItem->GetTitle());

		UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
		if (!GameInstance)
		{
			return;
		}

		const bool bEquipped = GameInstance->IsItemEquippedBySku(
			GetOwningPlayer()->GetLocalPlayer()->GetControllerId(),
			EItemSkuPlatform::AccelByte,
			Item->GetSkuMap()[EItemSkuPlatform::AccelByte]);

		SwitchActionButton(!bEquipped);
	}
}

void UInventoryWidget::OnClickEquip() const
{
	if (!SelectedItem)
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		return;
	}

	/**
	 * Item with bConsumable == false will always have its count as 0.
	 * Byte Wars expects the item to have count as at least 1 to be treated as equipped.
	 * Therefore, we manually override item count here.
	 */
	const TTuple<FString, int32> EquipItemData = {SelectedItem->GetSkuMap()[EItemSkuPlatform::AccelByte], SelectedItem->GetIsConsumable() ? SelectedItem->GetCount() : 1};
	const bool bSucceeded = GameInstance->UpdateEquippedItemsBySku(
		GetOwningPlayer()->GetLocalPlayer()->GetControllerId(),
		EItemSkuPlatform::AccelByte,
		{EquipItemData});

	SwitchActionButton(!bSucceeded);
	OnEquipped.Broadcast();
}

void UInventoryWidget::OnClickUnEquip() const
{
	if (!SelectedItem)
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	GameInstance->UnEquipItemBySku(
		GetOwningPlayer()->GetLocalPlayer()->GetControllerId(),
		EItemSkuPlatform::AccelByte,
		SelectedItem->GetSkuMap()[EItemSkuPlatform::AccelByte]);

	SwitchActionButton(true);
	OnUnequipped.Broadcast();
}
#pragma endregion 

#pragma region "UI"
void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector MenuViewTarget(60.0f, 750.0f, 160.0f);
	MoveCameraToTargetLocation(InDeltaTime, MenuViewTarget);
}

UWidget* UInventoryWidget::NativeGetDesiredFocusTarget() const
{
	return Tv_Content->GetListItems().IsEmpty() ? static_cast<UWidget*>(Btn_Back) : static_cast<UWidget*>(Tv_Content);
}

void UInventoryWidget::SwitchActionButton(bool bShowEquipButton) const
{
	Btn_Equip->SetVisibility(bShowEquipButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Btn_Unequip->SetVisibility(bShowEquipButton ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UInventoryWidget::SwitchToDefaultState() const
{
	/**
	 * bIsConsumable is used to differentiate power up and skin item in Byte Wars
	 * Skin will have a video preview, and power up will have an image preview
	 */
	if (bIsConsumable)
	{
		MediaPlayer->Close();
	}
	else
	{
		W_SelectedItemPreview->SetBrush(DefaultPreviewImage);
	}
	Tb_SelectedItemTitle->SetText(TEXT_NOTHING_SELECTED);
	Tb_Description->SetText(FText::FromString(TEXT("")));
	Tb_Description->SetText(FText::FromString(TEXT("")));
}
#pragma endregion
