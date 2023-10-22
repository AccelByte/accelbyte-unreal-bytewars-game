// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InventoryWidget.h"

#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Components/TileView.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"
#include "TutorialModules/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"

void UInventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	// bind
	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.AddUObject(this, &ThisClass::ShowEntitlements);
	Tv_Content->OnItemClicked().AddUObject(this, &UInventoryWidget::OnClickListItem);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Equip->OnClicked().AddUObject(this, &ThisClass::OnClickEquip);

	// reset UI
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	if (!EntitlementsSubsystem->GetIsQueryRunning())
	{
		EntitlementsSubsystem->QueryUserEntitlement(GetOwningPlayer());
	}

	UItemDataObject* EmptyData = NewObject<UItemDataObject>();
	EmptyData->Title = TEXT_NOTHING_SELECTED;
	SelectedItem = EmptyData;
	W_SelectedItem_Preview->LoadImage(SelectedItem->IconUrl);
	Tb_SelectedItem_Title->SetText(SelectedItem->Title);
	Btn_Equip->SetIsInteractionEnabled(false);
}

void UInventoryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.RemoveAll(this);
	Tv_Content->OnItemClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Equip->OnClicked().RemoveAll(this);
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	if (OnInventorysMenuDeactivated.IsBound())
	{
		OnInventorysMenuDeactivated.Broadcast(GetOwningPlayer());
	}
}

void UInventoryWidget::ShowEntitlements(const FOnlineError& Error, const TArray<UItemDataObject*> Entitlements) const
{
	if (Error.bSucceeded)
	{
		// filter items
		TArray<UItemDataObject*> FilteredEntitlements = Entitlements;
		FilteredEntitlements.RemoveAll([this](const UItemDataObject* Item)
		{
			return bIsConsumable != Item->bConsumable;
		});

		Tv_Content->ClearListItems();
		Tv_Content->SetListItems(FilteredEntitlements);
		Ws_Root->SetWidgetState(Tv_Content->GetListItems().IsEmpty() ?
			EAccelByteWarsWidgetSwitcherState::Empty :
			EAccelByteWarsWidgetSwitcherState::Not_Empty);

		Tv_Content->SetUserFocus(GetOwningPlayer());
	}
	else
	{
		Ws_Root->ErrorMessage = Error.ErrorMessage;
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 750.0f, 160.0f));
}

UWidget* UInventoryWidget::NativeGetDesiredFocusTarget() const
{
	if (Tv_Content->GetListItems().IsEmpty())
	{
		return Btn_Back;
	}
	else
	{
		return Tv_Content;
	}
}

void UInventoryWidget::OnClickListItem(UObject* Object)
{
	if (UItemDataObject* Item = Cast<UItemDataObject>(Object))
	{
		SelectedItem = Item;
		W_SelectedItem_Preview->LoadImage(SelectedItem->IconUrl);
		Tb_SelectedItem_Title->SetText(SelectedItem->Title);

		Btn_Equip->SetIsInteractionEnabled(true);
	}
}

void UInventoryWidget::OnClickEquip()
{
	if (SelectedItem == nullptr)
		return;

	UAccelByteWarsGameInstance* ABGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (ABGameInstance == nullptr)
		return;

	// Store -> Ship Item Lookup
	if (SelectedItem->Id == "4f6a077395214cabade85e53f47b7a7c") // Default Triangle Ship
	{
		ABGameInstance->SetShipSelection((int32)ShipDesign::TRIANGLE);
	}
	else if (SelectedItem->Id == "49bb99d9b20e48759bf6784bc640a936") // D Ship
	{
		ABGameInstance->SetShipSelection((int32)ShipDesign::D);
	}
	else if (SelectedItem->Id == "112c59e29b1f4d34b782142b4e53a540") // Double Triangle
	{
		ABGameInstance->SetShipSelection((int32)ShipDesign::DOUBLE_TRIANGLE);
	}
	else if (SelectedItem->Id == "ad8bd8a02a604796b3c9c5665e36bc1b") // Glow Xtra
	{
		ABGameInstance->SetShipSelection((int32)ShipDesign::GLOW_XTRA);
	}
	else if (SelectedItem->Id == "a758d7fb7147465cbd4fb83b2b53d29d") // White Star
	{
		ABGameInstance->SetShipSelection((int32)ShipDesign::WHITE_STAR);;
	}
}
