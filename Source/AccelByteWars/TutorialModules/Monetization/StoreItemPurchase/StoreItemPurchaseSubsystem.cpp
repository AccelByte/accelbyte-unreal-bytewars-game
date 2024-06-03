// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPurchaseSubsystem.h"

void UStoreItemPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlinePurchasePtr PurchasePtr = Online::GetSubsystem(GetWorld())->GetPurchaseInterface();
	ensure(PurchasePtr);

	PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(PurchasePtr);
	ensure(PurchaseInterface);

	PurchaseInterface->OnCreateNewOrderCompleteDelegates.AddUObject(this, &ThisClass::OnCreateNewOrderComplete);
}

void UStoreItemPurchaseSubsystem::Deinitialize()
{
	Super::Deinitialize();

	PurchaseInterface->ClearOnCreateNewOrderCompleteDelegates(this);
}

void UStoreItemPurchaseSubsystem::CreateNewOrder(
	const APlayerController* OwningPlayer,
	const UStoreItemDataObject* StoreItemData,
	const int32 SelectedPriceIndex,
	const int32 Quantity) const
{
	UStoreItemPriceDataObject* SelectedPrice = StoreItemData->Prices[SelectedPriceIndex];
	FAccelByteModelsOrderCreate Order{
		StoreItemData->ItemData->Id,
		Quantity,
		static_cast<int32>(SelectedPrice->RegularPrice) * Quantity,
		static_cast<int32>(SelectedPrice->FinalPrice) * Quantity,
		CurrencyCodeMap[SelectedPrice->CurrencyType]
	};

	PurchaseInterface->CreateNewOrder(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		Order);
}

void UStoreItemPurchaseSubsystem::OnCreateNewOrderComplete(
	bool bWasSuccessful,
	const FAccelByteModelsOrderInfo& OrderInfo,
	const FOnlineErrorAccelByte& OnlineError) const
{
	OnCheckoutCompleteDelegates.Broadcast(OnlineError);
}

#pragma region "Utilities"
FUniqueNetIdPtr UStoreItemPurchaseSubsystem::GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const
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
