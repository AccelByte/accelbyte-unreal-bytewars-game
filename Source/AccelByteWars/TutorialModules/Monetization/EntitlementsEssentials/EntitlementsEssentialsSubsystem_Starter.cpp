// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem_Starter.h"

#include "OnlineSubsystemUtils.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/System/AccelByteWarsGameInstance.h"

void UEntitlementsEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ensure(Subsystem))
	{
		return;
	}

	IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	CloudSaveInterface = StaticCastSharedPtr<FOnlineCloudSaveAccelByte>(Subsystem->GetCloudSaveInterface());
	EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (!ensure(IdentityInterface) || !ensure(CloudSaveInterface) || !ensure(EntitlementsInterface))
	{
		return;
	}

	// TODO: Add your code here.
}

void UEntitlementsEssentialsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Add your code here.
}

#pragma region Module Entitlement Essentials Function Definitions
// TODO: Add your function definitions here.
#pragma endregion

#pragma region "Utilities"
TArray<UStoreItemDataObject*> UEntitlementsEssentialsSubsystem_Starter::EntitlementsToDataObjects(
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const
{
	TArray<UStoreItemDataObject*> Items;
	for (const TSharedRef<FOnlineEntitlement>& Entitlement : Entitlements)
	{
		Items.Add(EntitlementToDataObject(Entitlement));
	}
	return Items;
}

UStoreItemDataObject* UEntitlementsEssentialsSubsystem_Starter::EntitlementToDataObject(
	TSharedRef<FOnlineEntitlement> Entitlement) const
{
	UStoreItemDataObject* EntitlementItem = NewObject<UStoreItemDataObject>();
	EntitlementItem->Setup(Entitlement.Get());
	EntitlementItem->SetIsShowPrices(false);

	if (TSharedPtr<FOnlineStoreOffer> StoreOffer = StoreInterface->GetOffer(Entitlement->ItemId))
	{
		if (UStoreItemDataObject* StoreItem = FInGameStoreEssentialsUtils::ConvertStoreData(StoreOffer.ToSharedRef().Get()))
		{
			EntitlementItem->UpdateVariables(StoreItem);
		}
	}

	return EntitlementItem;
}
#pragma endregion 
