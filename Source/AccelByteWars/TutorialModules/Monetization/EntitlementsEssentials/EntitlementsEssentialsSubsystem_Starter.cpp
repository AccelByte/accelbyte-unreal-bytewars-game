// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem_Starter.h"

#include "EntitlementsEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "TutorialModules/Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem_Starter.h"

#define STORE_SUBSYSTEM_CLASS UInGameStoreEssentialsSubsystem_Starter

void UEntitlementsEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		return;
	}

	EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (!ensure(EntitlementsInterface)) 
	{
		return;
	}

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 
}

void UEntitlementsEssentialsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion 
}

#pragma region "Tutorial"
// Put your code here.
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
	UStoreItemDataObject* Item = NewObject<UStoreItemDataObject>();
	Item->Setup(Entitlement.Get());

	// Byte Wars specifics, used to hide the prices on the UI
	Item->SetShouldShowPrices(false);

	for (const UStoreItemDataObject* Offer : StoreOffers)
	{
		if (Offer->GetStoreItemId().Equals(Item->GetStoreItemId()))
		{
			Item->UpdateVariables(Offer);
			break;
		}
	}

	return Item;
}

FUniqueNetIdPtr UEntitlementsEssentialsSubsystem_Starter::GetLocalPlayerUniqueNetId(
	const APlayerController* PlayerController) const
{
	if (!PlayerController) 
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
#pragma endregion 

#undef STORE_SUBSYSTEM_CLASS
