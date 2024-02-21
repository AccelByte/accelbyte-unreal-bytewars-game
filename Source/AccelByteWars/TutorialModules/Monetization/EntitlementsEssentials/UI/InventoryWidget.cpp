// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InventoryWidget.h"
#include "Components/TileView.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "Core/Ships/PlayerShipModels.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"

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
	Btn_Unequip->OnClicked().AddUObject(this, &ThisClass::OnClickUnequip);
	if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		Btn_Unequip->SetIsEnabled(GameInstance->GetShipPowerUp() != EPowerUpSelection::NONE);
	}

	// reset UI
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	if (!EntitlementsSubsystem->GetIsQueryRunning())
	{
		if (const APlayerController* PC = GetOwningPlayer();
			const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer()) 
		{
			EntitlementsSubsystem->QueryUserEntitlement(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId());
		}
	}

	UItemDataObject* EmptyData = NewObject<UItemDataObject>();
	EmptyData->Title = TEXT_NOTHING_SELECTED;
	SelectedItem = EmptyData;
	W_SelectedItem_Preview->LoadImage(SelectedItem->IconUrl);
	Tb_SelectedItem_Title->SetText(SelectedItem->Title);
	Btn_Equip->SetIsEnabled(false);

	if (OnInventoryMenuActivated.IsBound())
	{
		OnInventoryMenuActivated.Broadcast(GetOwningPlayer(), TDelegate<void()>::CreateWeakLambda(this, [this]()
		{
			Tv_Content->RegenerateAllEntries();
		}));
	}
}

void UInventoryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	EntitlementsSubsystem->OnQueryUserEntitlementsCompleteDelegates.RemoveAll(this);
	Tv_Content->OnItemClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Equip->OnClicked().RemoveAll(this);
	Btn_Unequip->OnClicked().RemoveAll(this);
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	if (OnInventoryMenuDeactivated.IsBound())
	{
		OnInventoryMenuDeactivated.Broadcast(GetOwningPlayer(), TDelegate<void()>());
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
			return bIsConsumable != Item->bConsumable || (Item->bConsumable && Item->Count <= 0);
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

		Btn_Equip->SetIsEnabled(true);
	}
}

void UInventoryWidget::OnClickEquip()
{
	if (SelectedItem == nullptr) 
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		return;
	}

	Btn_Unequip->SetIsEnabled(true);

	// Power Ups Selection
	const EPowerUpSelection PowerUp = ConvertItemIdToPowerUp(SelectedItem->Id);
	if (PowerUp != EPowerUpSelection::NONE)
	{
		GameInstance->SetShipPowerUp((int32)PowerUp);
		GameInstance->SaveGameSettings();
		Tv_Content->RegenerateAllEntries();
		return;
	}

	// Ship Selection
	GameInstance->SetShipSelection((int32)ConvertItemIdToShipDesign(SelectedItem->Id));
	GameInstance->SaveGameSettings();
	Tv_Content->RegenerateAllEntries();
}

void UInventoryWidget::OnClickUnequip()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	Btn_Unequip->SetIsEnabled(false);

	GameInstance->SetShipPowerUp((int32)EPowerUpSelection::NONE);
	GameInstance->SaveGameSettings();

	Tv_Content->RegenerateAllEntries();
}
