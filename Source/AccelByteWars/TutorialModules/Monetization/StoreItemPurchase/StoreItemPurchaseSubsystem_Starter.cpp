// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPurchaseSubsystem_Starter.h"
#include "OnlinePurchaseInterfaceAccelByte.h"

void UStoreItemPurchaseSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlinePurchasePtr PurchasePtr = Online::GetSubsystem(GetWorld())->GetPurchaseInterface();
	ensure(PurchasePtr);

	PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(PurchasePtr);
	ensure(PurchaseInterface);

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}

void UStoreItemPurchaseSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}

#pragma region "Tutorial"
// put your code here
#pragma endregion 

#pragma region "Utilities"
FUniqueNetIdPtr UStoreItemPurchaseSubsystem_Starter::GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const
{
	if (!ensure(PlayerController)) 
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
#pragma endregion
