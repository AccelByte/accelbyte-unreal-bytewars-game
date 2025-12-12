// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "EquipmentInventoryWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/Ships/PlayerShipBase.h"

#include "Kismet/GameplayStatics.h"
#include "CommonButtonBase.h"

// @@@SNIPSTART EquipmentInventoryWidget.cpp-NativeOnActivated
// @@@MULTISNIP Subsystem {"selectedLines": ["1-2", "5-6", "17"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "8-17"]}
void UEquipmentInventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem>();
	ensure(EntitlementsSubsystem);

	W_Inventory->OnCategorySwitched.AddUObject(this, &ThisClass::DisplayEquipmentItems);
	W_Inventory->OnItemClicked.AddUObject(this, &ThisClass::OnEquipmentItemClicked);
	W_Inventory->OnItemEquipClicked.AddUObject(this, &ThisClass::OnEquipmentItemEquipped);
	W_Inventory->OnItemUnequipClicked.AddUObject(this, &ThisClass::OnEquipmentItemUnequipped);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::SaveEquipments);

	SpawnPreviewActor();

	QueryEquipmentItems();
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-NativeOnDeactivated
void UEquipmentInventoryWidget::NativeOnDeactivated()
{
	W_Inventory->OnCategorySwitched.Clear();
	W_Inventory->OnItemClicked.Clear();
	W_Inventory->OnItemEquipClicked.Clear();
	W_Inventory->OnItemUnequipClicked.Clear();
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-SpawnPreviewActor
void UEquipmentInventoryWidget::SpawnPreviewActor()
{
	if (PreviewActorClass && !PreviewActor)
	{
		// Try to get existing preview actor.
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClassWithTag(this, PreviewActorClass, { PreviewActorTag }, Actors);
		if (!Actors.IsEmpty() && Actors[0]->IsValidLowLevel())
		{
			PreviewActor = Actors[0];
		}
		
		// Otherwise, try to spawn a new preview actor.
		if (!PreviewActor && GetWorld())
		{
			PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewActorClass, PreviewActorTransform);
			if (PreviewActor)
			{
				PreviewActor->Tags.Add(PreviewActorTag);
			}
		}
		ensureMsgf(PreviewActor, TEXT("Preview actor is not found. Failed to spawn or find existing actor."));
	}

	if (PreviewActor)
	{
		W_Inventory->SetPreview(PreviewActorRenderTarget);
	}
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-QueryEquipmentItems
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-13", "49"]}
void UEquipmentInventoryWidget::QueryEquipmentItems()
{
	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to save customization. Local player is invalid.");
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		return;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	const FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Get the equipment items.
	EntitlementsSubsystem->GetUserEquipments(
		LocalUserNum,
		UserId,
		FOnUpdateUserEquipmentsComplete::CreateWeakLambda(this, [this, UserId]
		(const FOnlineError& Error, const FPlayerEquipments& Equipments)
		{
			// Store current equipments if any.
			if (Error.bSucceeded)
			{
				CachedEquipments = Equipments;
			}

			// Query entitlements to be displayed.
			EntitlementsSubsystem->GetOrQueryUserEntitlements(
				UserId,
				FOnGetOrQueryUserEntitlementsComplete::CreateWeakLambda(this, [this]
				(const FOnlineError& Error, const TArray<UStoreItemDataObject*> Entitlements)
				{
					if (!Error.bSucceeded)
					{
						Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
						return;
					}

					CachedEntitlements.Empty();
					CachedEntitlements.Append(Entitlements);

					W_Inventory->RefreshCategories();
					W_Inventory->SwitchCategory();

					Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
				}));
		}));
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-DisplayEquipmentItems
void UEquipmentInventoryWidget::DisplayEquipmentItems(FInventoryCategory* Category)
{
	if (!Category)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to display customization items. Category is invalid.");
		return;
	}

	const FString CategoryId = Category->CategoryId.ToString();
	const EItemSkuPlatform PlatformSku = EItemSkuPlatform::AccelByte;
	TArray<UStoreItemDataObject*> FilteredEntitlements = CachedEntitlements;

	UObject* EquippedItemData = nullptr;
	const FString EquippedItemId = [&]() -> FString
	{
		switch (Category->ItemType)
		{
		case EItemType::Skin:
			return CachedEquipments.SkinId;
		case EItemType::Color:
			return CachedEquipments.ColorId;
		case EItemType::ExplosionFx:
			return CachedEquipments.ExplosionFxId;
		case EItemType::MissileTrailFx:
			return CachedEquipments.MissileTrailFxId;
		case EItemType::PowerUp:
			return CachedEquipments.PowerUpId;
		default:
			return FString();
		}
	}();
	
	FString EquippedItemSku = TEXT("");
	if (const UInGameItemDataAsset* EquippedItemDataAsset = UInGameItemUtility::GetItemDataAsset(EquippedItemId)) 
	{
		EquippedItemSku = EquippedItemDataAsset->SkuMap.Contains(PlatformSku) ? EquippedItemDataAsset->SkuMap[PlatformSku] : TEXT("");
	}

	// If there is a default item, add it as the first item in the list.
	if (const UInGameItemDataAsset* DefaultItemDataAsset = Category->DefaultItemDataAsset)
	{
		if (UStoreItemDataObject* DefaultItemData = NewObject<UStoreItemDataObject>())
		{
			DefaultItemData->Setup(
				DefaultItemDataAsset->DisplayName,
				FText::FromName(Category->CategoryId),
				UEnum::GetValueAsString(Category->ItemType),
				DefaultItemDataAsset->SkuMap[PlatformSku],
				TEXT(""),
				DefaultItemDataAsset->Icon->GetPathName(),
				DefaultItemDataAsset->SkuMap,
				{}, 1, false);

			EquippedItemData = DefaultItemData;
			FilteredEntitlements.Insert(DefaultItemData, 0);
		}
	}

	// Filter entitlements based on category.
	FilteredEntitlements.RemoveAll(
		[CategoryId, EquippedItemSku, PlatformSku, &EquippedItemData](UStoreItemDataObject* Item)
		{
			if (!Item)
			{
				return false;
			}

			const bool bIsValid = Item->GetCategory().ToString().Contains(CategoryId);
			if (bIsValid) 
			{
				if (Item->GetSkuMap().Contains(PlatformSku) && Item->GetSkuMap()[PlatformSku].Equals(EquippedItemSku))
				{
					EquippedItemData = Item;
				}
				Item->SetIsShowItemCount(Item->GetIsConsumable());
			}
			return !bIsValid;
		});

	// Initialize preview actor based on equipped items.
	if (APlayerShipBase* ShipPreview = Cast<APlayerShipBase>(PreviewActor))
	{
		if (const UInGameItemDataAsset* SkinDataAsset = UInGameItemUtility::GetItemDataAsset(CachedEquipments.SkinId))
		{
			ShipPreview->SetAlphaTexture(SkinDataAsset->Icon);
		}
		if (const UInGameItemDataAsset* ColorDataAsset = UInGameItemUtility::GetItemDataAsset(CachedEquipments.ColorId))
		{
			ShipPreview->SetColorTexture(ColorDataAsset->Icon);
		}
	}

	// Set initial equipped item and update the list items.
	if (EquippedItemData)
	{
		Category->EquippedItem = EquippedItemData;
	}
	W_Inventory->SetListItems(FilteredEntitlements);
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-OnEquipmentItemClicked
void UEquipmentInventoryWidget::OnEquipmentItemClicked(UObject* Item, FInventoryCategory* Category)
{
	const UStoreItemDataObject* ItemData = Cast<UStoreItemDataObject>(Item);
	if (!Category || !ItemData)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed handle item clicked event. Item is invalid.");
		return;
	}

	const EItemSkuPlatform PlatformSku = EItemSkuPlatform::AccelByte;
	const UInGameItemDataAsset* ItemDataAsset = 
		UInGameItemUtility::GetItemDataAssetBySku(PlatformSku, ItemData->GetSkuMap()[PlatformSku]);
	if (!ItemDataAsset)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed handle item clicked event. Item data asset is invalid.");
		return;
	}

	// Display preview based on item type.
	W_Inventory->SetPreviewDetails(ItemDataAsset->DisplayName, ItemDataAsset->Description);
	switch (Category->ItemType)
	{
	case EItemType::Skin:
		if (APlayerShipBase* ShipPreview = Cast<APlayerShipBase>(PreviewActor))
		{
			ShipPreview->SetAlphaTexture(ItemDataAsset->Icon);
		}
		break;
	case EItemType::Color:
		if (APlayerShipBase* ShipPreview = Cast<APlayerShipBase>(PreviewActor))
		{
			ShipPreview->SetColorTexture(ItemDataAsset->Icon);
		}
		break;
	default:
		if (ItemDataAsset->PreviewVideo) 
		{
			W_Inventory->SetPreview(ItemDataAsset->PreviewVideo);
		}
		break;
	}
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-OnEquipmentItemEquipped
void UEquipmentInventoryWidget::OnEquipmentItemEquipped(UObject* Item, FInventoryCategory* Category)
{
	const EItemSkuPlatform PlatformSku = EItemSkuPlatform::AccelByte;
	const UStoreItemDataObject* ItemData = Cast<UStoreItemDataObject>(Item);
	if (!Category || !ItemData || !ItemData->GetSkuMap().Contains(PlatformSku))
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed handle item unequipped event. Item is invalid.");
		return;
	}

	const UInGameItemDataAsset* ItemDataAsset =
		UInGameItemUtility::GetItemDataAssetBySku(PlatformSku, ItemData->GetSkuMap()[PlatformSku]);
	if (!ItemDataAsset)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed handle item unequipped event. Item data asset is invalid.");
		return;
	}

	// Update cached equipments based on item type.
	const FString& ItemId = ItemDataAsset->Id;
	switch (Category->ItemType)
	{
		case EItemType::Skin:
			CachedEquipments.SkinId = ItemId;
			break;
		case EItemType::Color:
			CachedEquipments.ColorId = ItemId;
			break;
		case EItemType::ExplosionFx:
			CachedEquipments.ExplosionFxId = ItemId;
			break;
		case EItemType::MissileTrailFx:
			CachedEquipments.MissileTrailFxId = ItemId;
			break;
		case EItemType::PowerUp:
			CachedEquipments.PowerUpId = ItemId;
			break;
	}
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-OnEquipmentItemUnequipped
void UEquipmentInventoryWidget::OnEquipmentItemUnequipped(UObject* Item, FInventoryCategory* Category)
{
	if (!Category)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed handle item equipped event. Item is invalid.");
		return;
	}

	// Unequip cached equipments based on item type.
	switch (Category->ItemType)
	{
	case EItemType::Skin:
		CachedEquipments.SkinId = TEXT("");
		break;
	case EItemType::Color:
		CachedEquipments.ColorId = TEXT("");
		break;
	case EItemType::ExplosionFx:
		CachedEquipments.ExplosionFxId = TEXT("");
		break;
	case EItemType::MissileTrailFx:
		CachedEquipments.MissileTrailFxId = TEXT("");
		break;
	case EItemType::PowerUp:
		CachedEquipments.PowerUpId = TEXT("");
		break;
	}
}
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.cpp-SaveEquipments
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-26", "41"]}
void UEquipmentInventoryWidget::SaveEquipments()
{
	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to save customization. Local player is invalid.");
		return;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to save customization. Game instance is invalid.");
		return;
	}

	UPromptSubsystem* PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	if (!PromptSubsystem)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to save customization. Prompt subsystem is invalid.");
		return;
	}

	PromptSubsystem->ShowLoading(TEXT_SAVING_EQUIPMENTS);
	EntitlementsSubsystem->SetUserEquipments(
		LocalUserNum,
		LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		CachedEquipments,
		FOnUpdateUserEquipmentsComplete::CreateWeakLambda(this, [this, PromptSubsystem]
		(const FOnlineError& Error, const FPlayerEquipments& Equipments)
		{
			PromptSubsystem->HideLoading();
			if (!Error.bSucceeded)
			{
				PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, Error.ErrorMessage);
			}
			DeactivateWidget();
		}));
}
// @@@SNIPEND