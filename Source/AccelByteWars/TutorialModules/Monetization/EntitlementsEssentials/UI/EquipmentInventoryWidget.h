// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/InventoryWidget.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem.h"
#include "EquipmentInventoryWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UCommonButtonBase;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UEquipmentInventoryWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART EquipmentInventoryWidget.h-public
// @@@MULTISNIP NativeOnActivated {"selectedLines": ["1-2"]}
// @@@MULTISNIP NativeOnDeactivated {"selectedLines": ["1", "3"]}
public:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
// @@@SNIPEND

// @@@SNIPSTART EquipmentInventoryWidget.h-protected
// @@@MULTISNIP Subsystem {"selectedLines": ["1", "37-38"]}
// @@@MULTISNIP EquipmentInventoryUI {"selectedLines": ["1", "13-44"]}
// @@@MULTISNIP SpawnPreviewActor {"selectedLines": ["1-2"]}
// @@@MULTISNIP QueryEquipmentItems {"selectedLines": ["1", "4"]}
// @@@MULTISNIP DisplayEquipmentItems {"selectedLines": ["1", "5"]}
// @@@MULTISNIP EquipmentItemEvents {"selectedLines": ["1", "7-9"]}
protected:
	void SpawnPreviewActor();

	void QueryEquipmentItems();
	void DisplayEquipmentItems(FInventoryCategory* Category);

	void OnEquipmentItemClicked(UObject* Item, FInventoryCategory* Category);
	void OnEquipmentItemEquipped(UObject* Item, FInventoryCategory* Category);
	void OnEquipmentItemUnequipped(UObject* Item, FInventoryCategory* Category);

	void SaveEquipments();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UInventoryWidget* W_Inventory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<AActor> PreviewActorClass;

	UPROPERTY(BlueprintReadOnly)
	AActor* PreviewActor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName PreviewActorTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FTransform PreviewActorTransform;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FSlateBrush PreviewActorRenderTarget;

	UPROPERTY()
	UEntitlementsEssentialsSubsystem* EntitlementsSubsystem;

	UPROPERTY()
	TArray<UStoreItemDataObject*> CachedEntitlements;

	UPROPERTY()
	FPlayerEquipments CachedEquipments;
// @@@SNIPEND
};
