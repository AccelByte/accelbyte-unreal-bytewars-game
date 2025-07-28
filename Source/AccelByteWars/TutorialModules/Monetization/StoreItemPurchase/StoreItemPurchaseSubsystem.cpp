// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "StoreItemPurchaseSubsystem.h"
#include "OnlinePurchaseInterfaceAccelByte.h"

// @@@SNIPSTART StoreItemPurchaseSubsystem.cpp-Initialize
// @@@MULTISNIP PurchaseInterface {"selectedLines": ["1-2", "5-9", "12"]}
void UStoreItemPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlinePurchasePtr PurchasePtr = Online::GetSubsystem(GetWorld())->GetPurchaseInterface();
	ensure(PurchasePtr);

	PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(PurchasePtr);
	ensure(PurchaseInterface);

	PurchaseInterface->OnCreateNewOrderCompleteDelegates.AddUObject(this, &ThisClass::OnCreateNewOrderComplete);
}
// @@@SNIPEND

// @@@SNIPSTART StoreItemPurchaseSubsystem.cpp-Deinitialize
void UStoreItemPurchaseSubsystem::Deinitialize()
{
	Super::Deinitialize();

	PurchaseInterface->ClearOnCreateNewOrderCompleteDelegates(this);
}
// @@@SNIPEND

// @@@SNIPSTART StoreItemPurchaseSubsystem.cpp-CreateNewOrder
void UStoreItemPurchaseSubsystem::CreateNewOrder(
	const APlayerController* OwningPlayer,
	const TWeakObjectPtr<UStoreItemDataObject> StoreItemData,
	const int32 SelectedPriceIndex,
	const int32 Quantity) const
{
	const UStoreItemPriceDataObject* SelectedPrice = StoreItemData->GetPrices()[SelectedPriceIndex];
	const FAccelByteModelsOrderCreate Order{
		StoreItemData->GetStoreItemId(),
		Quantity,
		static_cast<int32>(SelectedPrice->GetRegularPrice()) * Quantity,
		static_cast<int32>(SelectedPrice->GetFinalPrice()) * Quantity,
		FPreConfigCurrency::GetCodeFromType(SelectedPrice->GetCurrencyType())
	};

	PurchaseInterface->CreateNewOrder(
		*GetLocalPlayerUniqueNetId(OwningPlayer).Get(),
		Order);
}
// @@@SNIPEND

// @@@SNIPSTART StoreItemPurchaseSubsystem.cpp-OnCreateNewOrderComplete
void UStoreItemPurchaseSubsystem::OnCreateNewOrderComplete(
	bool bWasSuccessful,
	const FAccelByteModelsOrderInfo& OrderInfo,
	const FOnlineErrorAccelByte& OnlineError) const
{
	if (OnCheckoutCompleteDelegates.IsBound())
	{
		OnCheckoutCompleteDelegates.Broadcast(OnlineError);
	}
}
// @@@SNIPEND

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
