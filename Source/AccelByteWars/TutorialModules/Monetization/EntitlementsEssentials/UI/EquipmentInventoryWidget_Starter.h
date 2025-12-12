// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/InventoryWidget.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem_Starter.h"
#include "EquipmentInventoryWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UCommonButtonBase;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UEquipmentInventoryWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
protected:
#pragma region Module Entitlement Essentials Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

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
	UEntitlementsEssentialsSubsystem_Starter* EntitlementsSubsystem;

	UPROPERTY()
	TArray<UStoreItemDataObject*> CachedEntitlements;

	UPROPERTY()
	FPlayerEquipments CachedEquipments;
};
