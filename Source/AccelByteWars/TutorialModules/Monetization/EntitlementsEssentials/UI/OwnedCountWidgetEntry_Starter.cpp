// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "OwnedCountWidgetEntry_Starter.h"

#include "Components/TextBlock.h"
#include "Monetization/EntitlementsEssentials/EntitlementsEssentialsSubsystem_Starter.h"
#include "Core/UI/MainMenu/Store/Components/StoreItemListEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget.h"
#include "Monetization/StoreItemPurchase/UI/ItemPurchaseWidget_Starter.h"
#include "Monetization/EntitlementsEssentials/UI/EquipmentInventoryWidget.h"

#define PARENT_WIDGET_CLASS UEquipmentInventoryWidget

void UOwnedCountWidgetEntry_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	EntitlementsSubsystem = GetGameInstance()->GetSubsystem<UEntitlementsEssentialsSubsystem_Starter>();
	ensure(EntitlementsSubsystem);

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion
}

void UOwnedCountWidgetEntry_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 
}

#pragma region "Tutorial"
// Put your code here.
#pragma endregion 

#undef PARENT_WIDGET_CLASS
