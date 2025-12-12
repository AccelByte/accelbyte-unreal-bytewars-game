// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "EquipmentInventoryWidget_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/Ships/PlayerShipBase.h"

#include "Kismet/GameplayStatics.h"
#include "CommonButtonBase.h"

void UEquipmentInventoryWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem_Starter>();
	ensure(EntitlementsSubsystem);

	// TODO: Add your code here.
}

void UEquipmentInventoryWidget_Starter::NativeOnDeactivated()
{
	W_Inventory->OnCategorySwitched.Clear();
	W_Inventory->OnItemClicked.Clear();
	W_Inventory->OnItemEquipClicked.Clear();
	W_Inventory->OnItemUnequipClicked.Clear();
	Btn_Back->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

#pragma region Module Entitlement Essentials Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
